#!/usr/bin/env python3
"""
Extract all Memory::Scan(module, "pattern") calls from the gamesense_legacy
codebase and dump them as JSON: { id, file, line, module, pattern, wildcard_mask }.
"""
import re
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]  # /home/user/Gamesense-Legacy
SRC = ROOT / "gamesense_legacy"

# Matches: Memory::Scan( image_client, XorStr( "..." ) )  and variants
SCAN_RE = re.compile(
    r'Memory::Scan\(\s*'
    r'([^,]+?)'                                                    # module arg
    r'\s*,\s*'
    r'(?:XorStr\s*\(\s*)?"((?:\\.|[^"\\])*)"'                     # pattern string
)

# Also captures raw string literals used for scans done via other helpers
def unescape(s: str) -> str:
    return s.replace(r'\"', '"').replace(r'\\', '\\').replace(r'\n', '\n').replace(r'\t', '\t').replace(r'\r', '\r')

def parse_pattern(pat: str):
    """Split IDA-style pattern into bytes + wildcard mask."""
    tokens = pat.split()
    bytes_out = []
    mask = []
    for t in tokens:
        if '?' in t:
            bytes_out.append(0)
            mask.append(False)
        else:
            bytes_out.append(int(t, 16))
            mask.append(True)
    return bytes_out, mask

def main():
    results = []
    files = sorted(SRC.rglob('*.cpp')) + sorted(SRC.rglob('*.hpp')) + sorted(SRC.rglob('*.h'))
    for f in files:
        if 'Libraries' in f.parts:
            continue
        try:
            text = f.read_text(encoding='utf-8', errors='replace')
        except Exception:
            continue
        for m in SCAN_RE.finditer(text):
            line = text.count('\n', 0, m.start()) + 1
            pat = unescape(m.group(2))
            try:
                bts, mask = parse_pattern(pat)
            except ValueError:
                results.append({
                    'file': str(f.relative_to(ROOT)), 'line': line,
                    'module': m.group(1), 'pattern': pat, 'error': 'unparseable'
                })
                continue
            # context: the variable it is assigned to (previous ~200 chars)
            ctx_start = text.rfind('\n', 0, max(0, m.start() - 220))
            ctx = text[ctx_start:m.start()].strip().split('\n')[-1]
            results.append({
                'file': str(f.relative_to(ROOT)), 'line': line,
                'module': m.group(1), 'pattern': pat,
                'bytes': bts, 'mask': mask,
                'context': ctx[-160:],
            })
    out = Path(__file__).parent / 'patterns.json'
    out.write_text(json.dumps(results, indent=1))
    print(f"extracted {len(results)} scan calls -> {out}")
    mods = {}
    for r in results:
        mods[r['module']] = mods.get(r['module'], 0) + 1
    for k, v in sorted(mods.items(), key=lambda x: -x[1]):
        print(f"  {k}: {v}")

if __name__ == '__main__':
    main()
