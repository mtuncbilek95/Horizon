import re
import argparse
from pathlib import Path

def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    return text

def parse_attribute(content: str):
    m = re.match(r'\s*([A-Za-z_]\w*)\s*\[(.*)\]\s*$', content.strip(), flags=re.S)
    if not m:
        return None
    return m.group(1), m.group(2).strip()

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
    for hclass in re.finditer(r'HCLASS\s*\(([^)]*)\)', clean):
        m = re.search(r'\b(?:class|struct)\s+(?:[A-Z_][A-Z0-9_]*\s+)?([A-Za-z_]\w*)\b([^{;]*)\{', clean[hclass.end():])
        if not m:
            continue

        name = m.group(1)
        inherit = m.group(2)

        base = None
        bm = re.search(r':\s*(?:public\s+|protected\s+|private\s+)?([A-Za-z_][\w:]*)', inherit)
        if bm:
            base = bm.group(1)

        before = clean[:hclass.start()]
        ns_matches = re.findall(r'namespace\s+([A-Za-z_][\w:]*)', before)
        ns = ns_matches[-1] if ns_matches else ''

        body_open = hclass.end() + m.end() - 1
        body = brace_body(clean, body_open)

        attrs = []
        head_attr = parse_attribute(hclass.group(1)) if hclass.group(1).strip() else None
        if head_attr:
            attrs.append(head_attr)
        for a in re.finditer(r'HATTRIBUTE\s*\(([^)]*)\)', body):
            pa = parse_attribute(a.group(1))
            if pa:
                attrs.append(pa)

        fields = []
        for f in re.finditer(r'HFIELD\s*\(\s*\)\s*;?\s*([^;{}]+);', body):
            decl = f.group(1).strip()
            nm = re.search(r'([A-Za-z_]\w*)\s*$', decl)
            if nm:
                member = nm.group(1)
                display = member[2:] if member.startswith('m_') else member
                fields.append((display, member))

        rel = path.relative_to(source_root).as_posix()
        layer = rel.split('/', 1)[0]
        results.append(Reflected(ns, name, base, attrs, fields, rel, layer))

    return results

def emit_reflected(r: Reflected) -> str:
    q = f'{r.ns}::{r.name}' if r.ns else r.name
    lines = [f'\t\t\t\treturn TypeBuilder<{q}>::ForType("{r.name}")']
    if r.base:
        bq = f'{r.ns}::{r.base}' if r.ns else r.base
        lines.append(f'\t\t\t\t\t.WithBase<{bq}>()')
    for attr_name, args in r.attrs:
        aq = f'{r.ns}::{attr_name}' if r.ns else attr_name
        lines.append(f'\t\t\t\t\t.WithAttribute<{aq}>({args})')
    for display, member in r.fields:
        lines.append(f'\t\t\t\t\t.WithField("{display}", &{q}::{member})')
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
        f'\toutTypes->push_back(TypeAccessor<{(r.ns + "::" + r.name) if r.ns else r.name}>::Build());'
        for r in all_r
    )
    return (
        '#pragma once\n\n'
        f'{includes}\n\n'
        '#include <Runtime/Definitions/Allocator.h>\n\n'
        '#include <vector>\n\n'
        'extern "C" H_EXPORT void InstallModule(void* allocatorCtx)\n'
        '{\n'
        '\tHorizon::Allocator::SetContext(allocatorCtx);\n'
        '}\n\n'
        'extern "C" H_EXPORT void GenerateModuleManifestation(std::vector<Horizon::Reflect::Type>* outTypes)\n'
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