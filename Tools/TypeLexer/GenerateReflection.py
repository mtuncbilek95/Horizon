import re
import argparse
from pathlib import Path

def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    return text

def parse_attribute(content: str):
    m = re.match(r'\s*([A-Za-z_]\w*)\s*(?:\[(.*)\])?\s*$', content.strip(), flags=re.S)
    if not m:
        return None
    return m.group(1), (m.group(2) or '').strip()

def split_top_level(s: str):
    parts = []
    cur = []
    depth = 0
    in_str = None
    i = 0
    while i < len(s):
        ch = s[i]
        if in_str:
            cur.append(ch)
            if ch == '\\' and i + 1 < len(s):
                cur.append(s[i + 1])
                i += 1
            elif ch == in_str:
                in_str = None
        elif ch in '"\'':
            in_str = ch
            cur.append(ch)
        elif ch in '[({':
            depth += 1
            cur.append(ch)
        elif ch in '])}':
            depth -= 1
            cur.append(ch)
        elif ch == ',' and depth == 0:
            parts.append(''.join(cur).strip())
            cur = []
        else:
            cur.append(ch)
        i += 1
    tail = ''.join(cur).strip()
    if tail:
        parts.append(tail)
    return parts

def parse_attributes(content: str):
    attrs = []
    if not content.strip():
        return attrs
    for part in split_top_level(content):
        pa = parse_attribute(part)
        if pa:
            attrs.append(pa)
    return attrs

def attribute_class(name: str) -> str:
    return name if name.endswith('Attribute') else name + 'Attribute'

NS_TAIL_RE = re.compile(r'namespace\s+([A-Za-z_][\w:]*)\s*$')

def namespace_at(clean: str, pos: int) -> str:
    stack = []
    i = 0
    n = min(pos, len(clean))
    while i < n:
        ch = clean[i]
        if ch in '"\'':
            quote = ch
            i += 1
            while i < n:
                if clean[i] == '\\':
                    i += 2
                    continue
                if clean[i] == quote:
                    break
                i += 1
        elif ch == '{':
            m = NS_TAIL_RE.search(clean, 0, i)
            stack.append(m.group(1) if m else None)
        elif ch == '}':
            if stack:
                stack.pop()
        i += 1
    return '::'.join(name for name in stack if name)

def brace_body(s: str, open_idx: int) -> str:
    depth = 0
    for i in range(open_idx, len(s)):
        if s[i] == '{':
            depth += 1
        elif s[i] == '}':
            depth -= 1
            if depth == 0:
                return s[open_idx + 1:i]
    return s[open_idx + 1:]

def extract_parens(s: str, open_idx: int):
    depth = 0
    for i in range(open_idx, len(s)):
        if s[i] == '(':
            depth += 1
        elif s[i] == ')':
            depth -= 1
            if depth == 0:
                return s[open_idx + 1:i], i
    return s[open_idx + 1:], len(s) - 1

class Reflected:
    def __init__(self, ns, name, base, attrs, fields, include, layer):
        self.ns = ns
        self.name = name
        self.base = base
        self.attrs = attrs
        self.fields = fields
        self.include = include
        self.layer = layer

def parse_file(path: Path, source_root: Path):
    raw = path.read_text(encoding='utf-8', errors='ignore')
    clean = strip_comments(raw)

    results = []
    for hclass in re.finditer(r'\bHCLASS\s*\(', clean):
        paren_open = hclass.end() - 1
        hclass_content, paren_close = extract_parens(clean, paren_open)

        m = re.search(r'\b(?:class|struct)\s+(?:[A-Z_][A-Z0-9_]*\s+)?([A-Za-z_]\w*)\b([^{;]*)\{', clean[paren_close:])
        if not m:
            continue

        name = m.group(1)
        inherit = m.group(2)

        base = None
        bm = re.search(r':\s*(?:public\s+|protected\s+|private\s+)?([A-Za-z_][\w:]*)', inherit)
        if bm:
            base = bm.group(1)

        ns = namespace_at(clean, hclass.start())

        body_open = paren_close + m.end() - 1
        body = brace_body(clean, body_open)

        attrs = parse_attributes(hclass_content)
        for a in re.finditer(r'\bHATTRIBUTE\s*\(', body):
            a_content, _ = extract_parens(body, a.end() - 1)
            attrs.extend(parse_attributes(a_content))

        fields = []
        for f in re.finditer(r'\bHFIELD\s*\(', body):
            f_content, f_close = extract_parens(body, f.end() - 1)

            decl_m = re.match(r'\s*;?\s*([^;{}]+);', body[f_close + 1:])
            if not decl_m:
                continue

            decl = decl_m.group(1).split('=')[0].strip()
            nm = re.search(r'([A-Za-z_]\w*)\s*$', decl)
            if not nm:
                continue

            member = nm.group(1)
            display = member[2:] if member.startswith('m_') else member
            fields.append((display, member, parse_attributes(f_content)))

        rel = path.relative_to(source_root).as_posix()
        layer = rel.split('/', 1)[0]
        results.append(Reflected(ns, name, base, attrs, fields, rel, layer))

    return results

def emit_reflected(r: Reflected) -> str:
    q = f'{r.ns}::{r.name}' if r.ns else r.name
    lines = []
    if r.ns:
        lines.append(f'\t\t\t\tusing namespace {r.ns};\n')
    lines.append(f'\t\t\t\treturn TypeBuilder<{r.name}>::ForType("{r.name}")')
    if r.base:
        lines.append(f'\t\t\t\t\t.WithBase<{r.base}>()')
    for attr_name, args in r.attrs:
        cls = attribute_class(attr_name)
        lines.append(f'\t\t\t\t\t.WithAttribute({cls}({args}))')
    for display, member, f_attrs in r.fields:
        lines.append(f'\t\t\t\t\t.WithField("{display}", &{r.name}::{member})')
        for attr_name, args in f_attrs:
            cls = attribute_class(attr_name)
            lines.append(f'\t\t\t\t\t.WithFieldAttribute({cls}({args}))')
    lines.append('\t\t\t\t\t.Build();')
    chain = '\n'.join(lines)

    return (
        '#pragma once\n\n'
        f'#include <{r.include}>\n\n'
        'namespace Horizon::Reflect\n{\n'
        '\ttemplate<>\n'
        f'\tstruct TypeAccessor<{q}>\n'
        '\t{\n'
        '\t\tstatic Type Build()\n'
        '\t\t{\n'
        f'{chain}\n'
        '\t\t}\n'
        '\t};\n'
        '}\n'
    )

def emit_manifestation(all_r) -> str:
    includes = '\n'.join(f'#include <{r.layer}/{r.name}.reflected.h>' for r in all_r)
    pushes = '\n'.join(
        f'\toutTypes->PushBack(TypeAccessor<{(r.ns + "::" + r.name) if r.ns else r.name}>::Build());'
        for r in all_r
    )
    return (
        '#pragma once\n\n'
        f'{includes}\n\n'
        '#include <Runtime/RTTR/Reflection.h>\n'
        '#include <Runtime/Definitions/Allocator.h>\n'
        '#include <Runtime/Containers/List.h>\n\n'
        'extern "C" H_EXPORT void InstallModule(void* allocatorCtx)\n'
        '{\n'
        '\tHorizon::Memory::Allocator::SetContext(allocatorCtx);\n'
        '}\n\n'
        'extern "C" H_EXPORT void GenerateModuleManifestation(Horizon::List<Horizon::Reflect::Type>* outTypes)\n'
        '{\n'
        '\tif (!outTypes)\n'
        '\t\treturn;\n\n'
        '\tusing namespace Horizon::Reflect;\n\n'
        f'{pushes}\n'
        '}\n'
    )

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--source', default='Source')
    ap.add_argument('--out', default='Intermediate')
    args = ap.parse_args()

    source_root = Path(args.source).resolve()
    out_root = Path(args.out).resolve()

    all_r = []
    for path in source_root.rglob('*.h'):
        if 'HCLASS' not in path.read_text(encoding='utf-8', errors='ignore'):
            continue
        for r in parse_file(path, source_root):
            all_r.append(r)
            out_path = out_root / r.layer / f'{r.name}.reflected.h'
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(emit_reflected(r), encoding='utf-8', newline='\n')
            print(f'[gen] {r.ns}::{r.name} -> {out_path.relative_to(out_root)}')

    (out_root / 'TypeManifestation.h').write_text(emit_manifestation(all_r), encoding='utf-8', newline='\n')
    print(f'[gen] TypeManifestation.h ({len(all_r)} types)')

if __name__ == '__main__':
    main()