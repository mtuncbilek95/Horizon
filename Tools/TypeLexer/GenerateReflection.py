import re
import argparse
from dataclasses import dataclass
from pathlib import Path

blockCommentRe = re.compile(r'/\*.*?\*/', flags=re.S)
lineCommentRe = re.compile(r'//[^\n]*')

namespaceTailRe = re.compile(r'namespace\s+([A-Za-z_][\w:]*)\s*$')
attributeRe = re.compile(r'\s*([A-Za-z_]\w*)\s*(?:\[(.*)\])?\s*$', flags=re.S)

classDeclRe = re.compile(r'\b(enum\s+)?(?:class|struct)\s+(?:[A-Z_][A-Z0-9_]*\s+)?([A-Za-z_]\w*)\b([^{;]*)\{')
enumDeclRe = re.compile(r'\benum\s+(?:class\s+|struct\s+)?(?:[A-Z_][A-Z0-9_]*\s+)?([A-Za-z_]\w*)\b[^{;]*\{')

hclassRe = re.compile(r'\bHCLASS\s*\(')
hattributeRe = re.compile(r'\bHATTRIBUTE\s*\(')
hfieldRe = re.compile(r'\bHFIELD\s*\(')
henumRe = re.compile(r'\bHENUM\s*\(')
hmetaRe = re.compile(r'\bHMETA\s*\(')

baseRe = re.compile(r':\s*(?:public\s+|protected\s+|private\s+)?([A-Za-z_][\w:]*)')
fieldDeclRe = re.compile(r'\s*;?\s*([^;{}]+);')
fieldNameRe = re.compile(r'([A-Za-z_]\w*)\s*$')
enumeratorRe = re.compile(r'([A-Za-z_]\w*)\s*(?:=(.*))?$', flags=re.S)
metaStringRe = re.compile(r'\s*"((?:[^"\\]|\\.)*)"')
shiftRe = re.compile(r'<<|>>')


@dataclass
class ReflectedAttribute:
    name: str
    arguments: str

    # Renders this attribute as the C++ constructor expression the builder expects.
    def ToExpression(self):
        className = self.name if self.name.endswith('Attribute') else self.name + 'Attribute'
        return f'{className}({self.arguments})'


@dataclass
class ReflectedField:
    displayName: str
    memberName: str
    attributes: list


@dataclass
class ReflectedEnumValue:
    displayName: str
    memberName: str


@dataclass
class ReflectedType:
    namespaceName: str
    typeName: str
    includePath: str
    layerName: str

    # Returns the fully qualified C++ name of this type.
    def QualifiedName(self):
        if self.namespaceName:
            return f'{self.namespaceName}::{self.typeName}'
        return self.typeName


@dataclass
class ReflectedClass(ReflectedType):
    baseName: str
    attributes: list
    fields: list

    # Builds the TypeBuilder call chain that describes this class.
    def BuildChain(self):
        chain = []

        if self.baseName:
            chain.append(f'.WithBase<{self.baseName}>()')

        for attribute in self.attributes:
            chain.append(f'.WithAttribute({attribute.ToExpression()})')

        for reflectedField in self.fields:
            chain.append(f'.WithField("{reflectedField.displayName}", &{self.typeName}::{reflectedField.memberName})')

            for attribute in reflectedField.attributes:
                chain.append(f'.WithFieldAttribute({attribute.ToExpression()})')

        return chain


@dataclass
class ReflectedEnum(ReflectedType):
    values: list

    # Builds the TypeBuilder call chain that describes this enum.
    def BuildChain(self):
        return [f'.WithEnum("{value.displayName}", static_cast<i64>({self.typeName}::{value.memberName}))'
                for value in self.values]


# Removes block and line comments so commented out markup is never parsed.
def StripComments(text):
    text = blockCommentRe.sub('', text)
    return lineCommentRe.sub('', text)


# Advances past a string or character literal and returns the index right after it.
def SkipLiteral(text, index, limit):
    quote = text[index]
    index += 1

    while index < limit:
        if text[index] == '\\':
            index += 2
            continue

        if text[index] == quote:
            return index + 1

        index += 1

    return index


# Returns the index of the delimiter closing the one at openIndex, or the end of the text.
def MatchingIndex(text, openIndex, opener, closer):
    depth = 0

    for index in range(openIndex, len(text)):
        if text[index] == opener:
            depth += 1
        elif text[index] == closer:
            depth -= 1

            if depth == 0:
                return index

    return len(text)


# Returns the text between the brace at openIndex and its matching close brace.
def BraceBody(text, openIndex):
    return text[openIndex + 1:MatchingIndex(text, openIndex, '{', '}')]


# Returns the text inside the paren at openIndex plus the index of its matching close paren.
def ExtractParens(text, openIndex):
    closeIndex = MatchingIndex(text, openIndex, '(', ')')
    return text[openIndex + 1:closeIndex], closeIndex


# Splits a comma separated list, ignoring commas nested in brackets or literals.
def SplitTopLevel(text):
    parts = []
    current = []
    depth = 0
    quote = None
    index = 0

    while index < len(text):
        char = text[index]

        if quote:
            current.append(char)

            if char == '\\' and index + 1 < len(text):
                current.append(text[index + 1])
                index += 1
            elif char == quote:
                quote = None
        elif char in '"\'':
            quote = char
            current.append(char)
        elif char in '[({':
            depth += 1
            current.append(char)
        elif char in '])}':
            depth -= 1
            current.append(char)
        elif char == ',' and depth == 0:
            parts.append(''.join(current).strip())
            current = []
        else:
            current.append(char)

        index += 1

    parts.append(''.join(current).strip())

    return [part for part in parts if part]


# Parses one "Name[args]" usage into a reflected attribute.
def ParseAttribute(part):
    match = attributeRe.match(part.strip())

    if not match:
        return None

    return ReflectedAttribute(match.group(1), (match.group(2) or '').strip())


# Parses a whole macro payload into the list of attributes it declares.
def ParseAttributes(payload):
    if not payload.strip():
        return []

    parsed = [ParseAttribute(part) for part in SplitTopLevel(payload)]

    return [attribute for attribute in parsed if attribute]


# Rebuilds the enclosing namespace path for a position in the file.
def NamespaceAt(text, position):
    stack = []
    index = 0
    limit = min(position, len(text))

    while index < limit:
        char = text[index]

        if char in '"\'':
            index = SkipLiteral(text, index, limit)
            continue

        if char == '{':
            match = namespaceTailRe.search(text, 0, index)
            stack.append(match.group(1) if match else None)
        elif char == '}' and stack:
            stack.pop()

        index += 1

    return '::'.join(name for name in stack if name)


# Joins a namespace and a type name into a fully qualified name.
def QualifiedName(namespaceName, typeName):
    if namespaceName:
        return f'{namespaceName}::{typeName}'

    return typeName


# Turns a member name into the display name the editor shows.
def DisplayNameOf(memberName):
    displayName = memberName[2:] if memberName.startswith('m_') else memberName

    return displayName[:1].upper() + displayName[1:]


# Finds the first real class or struct declaration after a marker, skipping enum declarations.
def FindClassDeclaration(text, startIndex):
    for candidate in classDeclRe.finditer(text, startIndex):
        if candidate.group(1):
            continue

        return candidate

    return None


# Collects the attributes contributed by HATTRIBUTE macros anywhere in a class body.
def ParseBodyAttributes(body):
    attributes = []

    for marker in hattributeRe.finditer(body):
        payload, _ = ExtractParens(body, marker.end() - 1)
        attributes.extend(ParseAttributes(payload))

    return attributes


# Collects every HFIELD marked member of a class body.
def ParseFields(body, ownerName):
    fields = []

    for marker in hfieldRe.finditer(body):
        payload, payloadEnd = ExtractParens(body, marker.end() - 1)
        declaration = fieldDeclRe.match(body[payloadEnd + 1:])

        if not declaration:
            print(f'[warn] {ownerName} has an HFIELD with an unparsable declaration, skipped')
            continue

        nameMatch = fieldNameRe.search(declaration.group(1).split('=')[0].strip())

        if not nameMatch:
            print(f'[warn] {ownerName} has an HFIELD with no member name, skipped')
            continue

        memberName = nameMatch.group(1)
        fields.append(ReflectedField(DisplayNameOf(memberName), memberName, ParseAttributes(payload)))

    return fields


# Splits one enumerator into its member name, display name and initializer text.
def ParseEnumerator(entry):
    displayName = None
    marker = hmetaRe.search(entry)

    if marker:
        payload, payloadEnd = ExtractParens(entry, marker.end() - 1)
        stringMatch = metaStringRe.match(payload)

        if stringMatch:
            displayName = stringMatch.group(1)

        entry = entry[:marker.start()] + entry[payloadEnd + 1:]

    entry = entry.strip()

    if not entry:
        return None

    match = enumeratorRe.match(entry)

    if not match:
        return None

    memberName = match.group(1)
    initializer = (match.group(2) or '').strip()

    return memberName, displayName or memberName, initializer


# Collects an enum body's values, or None when it is a bit shifted flag enum.
def ParseEnumValues(body):
    values = []

    for entry in SplitTopLevel(body):
        parsed = ParseEnumerator(entry)

        if not parsed:
            continue

        memberName, displayName, initializer = parsed

        if shiftRe.search(initializer):
            return None

        values.append(ReflectedEnumValue(displayName, memberName))

    return values


# Collects every HCLASS marked type in a cleaned file.
def ParseClasses(text, includePath, layerName):
    results = []

    for marker in hclassRe.finditer(text):
        payload, payloadEnd = ExtractParens(text, marker.end() - 1)
        declaration = FindClassDeclaration(text, payloadEnd)

        if not declaration:
            continue

        typeName = declaration.group(2)
        namespaceName = NamespaceAt(text, marker.start())
        body = BraceBody(text, declaration.end() - 1)

        baseMatch = baseRe.search(declaration.group(3))

        results.append(ReflectedClass(
            namespaceName=namespaceName,
            typeName=typeName,
            includePath=includePath,
            layerName=layerName,
            baseName=baseMatch.group(1) if baseMatch else None,
            attributes=ParseAttributes(payload) + ParseBodyAttributes(body),
            fields=ParseFields(body, QualifiedName(namespaceName, typeName))))

    return results


# Collects every HENUM marked enum in a cleaned file, skipping bit shifted flag enums.
def ParseEnums(text, includePath, layerName):
    results = []

    for marker in henumRe.finditer(text):
        _, payloadEnd = ExtractParens(text, marker.end() - 1)
        declaration = enumDeclRe.search(text, payloadEnd)

        if not declaration:
            continue

        typeName = declaration.group(1)
        namespaceName = NamespaceAt(text, marker.start())
        qualified = QualifiedName(namespaceName, typeName)

        values = ParseEnumValues(BraceBody(text, declaration.end() - 1))

        if values is None:
            print(f'[skip] {qualified} is a bit shifted flag enum, HENUM ignored')
            continue

        if not values:
            print(f'[skip] {qualified} has no enumerators, HENUM ignored')
            continue

        results.append(ReflectedEnum(
            namespaceName=namespaceName,
            typeName=typeName,
            includePath=includePath,
            layerName=layerName,
            values=values))

    return results


# Parses one header into every reflected class and enum it declares.
def ParseFile(path, sourceRoot, raw):
    text = StripComments(raw)
    includePath = path.relative_to(sourceRoot).as_posix()
    layerName = includePath.split('/', 1)[0]

    return ParseClasses(text, includePath, layerName) + ParseEnums(text, includePath, layerName)


# Renders the TypeAccessor specialisation wrapping a type's TypeBuilder chain.
def EmitAccessor(reflected):
    lines = []

    if reflected.namespaceName:
        lines.append(f'\t\t\t\tusing namespace {reflected.namespaceName};\n')

    lines.append(f'\t\t\t\treturn TypeBuilder<{reflected.typeName}>::ForType("{reflected.typeName}")')
    lines.extend(f'\t\t\t\t\t{step}' for step in reflected.BuildChain())
    lines.append('\t\t\t\t\t.Build();')

    body = '\n'.join(lines)

    return (
        '#pragma once\n\n'
        f'#include <{reflected.includePath}>\n\n'
        'namespace Horizon::Reflect\n{\n'
        '\ttemplate<>\n'
        f'\tstruct TypeAccessor<{reflected.QualifiedName()}>\n'
        '\t{\n'
        '\t\tstatic Type Build()\n'
        '\t\t{\n'
        f'{body}\n'
        '\t\t}\n'
        '\t};\n'
        '}\n'
    )


# Renders the module wide header that registers every generated type.
def EmitManifestation(reflected):
    includes = '\n'.join(
        f'#include <{entry.layerName}/{entry.typeName}.reflected.h>' for entry in reflected)
    pushes = '\n'.join(
        f'\toutTypes->PushBack(TypeAccessor<{entry.QualifiedName()}>::Build());' for entry in reflected)

    return (
        '#pragma once\n\n'
        f'{includes}\n\n'
        '#include <Runtime/RTTR/Reflection.h>\n'
        '#include <Runtime/Definitions/Allocator.h>\n'
        '#include <Runtime/Containers/List.h>\n\n'
        'extern "C" H_EXPORT void GenerateModuleManifestation(Horizon::List<Horizon::Reflect::Type>* outTypes)\n'
        '{\n'
        '\tif (!outTypes)\n'
        '\t\treturn;\n\n'
        '\tusing namespace Horizon::Reflect;\n\n'
        f'{pushes}\n'
        '}\n'
    )

# Deletes generated headers left over from types that no longer exist in the source tree.
def PruneStale(outRoot, claimed):
    for path in outRoot.rglob('*.reflected.h'):
        if path in claimed:
            continue

        path.unlink()
        print(f'[del] {path.relative_to(outRoot)}')

# Generates one .reflected.h per marked type plus the module manifestation header.
def Main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', default='Source')
    parser.add_argument('--out', default='Intermediate')
    arguments = parser.parse_args()

    sourceRoot = Path(arguments.source).resolve()
    outRoot = Path(arguments.out).resolve()
    outRoot.mkdir(parents=True, exist_ok=True)

    generated = []
    claimed = {}

    for path in sourceRoot.rglob('*.h'):
        raw = path.read_text(encoding='utf-8', errors='ignore')

        if 'HCLASS' not in raw and 'HENUM' not in raw:
            continue

        for reflected in ParseFile(path, sourceRoot, raw):
            outPath = outRoot / reflected.layerName / f'{reflected.typeName}.reflected.h'
            qualified = reflected.QualifiedName()
            owner = claimed.get(outPath)

            if owner:
                print(f'[err] {qualified} collides with {owner}, skipped')
                continue

            claimed[outPath] = qualified
            generated.append(reflected)

            outPath.parent.mkdir(parents=True, exist_ok=True)
            outPath.write_text(EmitAccessor(reflected), encoding='utf-8', newline='\n')
            print(f'[gen] {qualified} -> {outPath.relative_to(outRoot)}')

    PruneStale(outRoot, claimed)

    (outRoot / 'TypeManifestation.h').write_text(
        EmitManifestation(generated), encoding='utf-8', newline='\n')
    print(f'[gen] TypeManifestation.h ({len(generated)} types)')

if __name__ == '__main__':
    Main()