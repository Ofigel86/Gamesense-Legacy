#!/usr/bin/env python3
"""
Scan a PE file (32-bit) for IDA-style byte patterns with wildcards.
Usage:
  python pe_scan.py <dll> "<pattern>"          # single pattern, prints offsets
  python pe_scan.py <dll> --json patterns.json # batch: check all patterns from extract_patterns.py
  python pe_scan.py <dll> --interfaces         # list embedded interface version strings
"""
import re
import json
import sys
import struct
from pathlib import Path


def load_pe(data: bytes):
    """Return list of (va, raw_offset, size) for executable sections + image base."""
    if data[:2] != b'MZ':
        raise ValueError('not a PE')
    e_lfanew = struct.unpack_from('<I', data, 0x3C)[0]
    if data[e_lfanew:e_lfanew + 4] != b'PE\0\0':
        raise ValueError('not a PE')
    coff = e_lfanew + 4
    machine, num_sections, _, _, _, opt_size, _ = struct.unpack_from('<HHIIIHH', data, coff)
    opt = coff + 20
    magic = struct.unpack_from('<H', data, opt)[0]
    if magic == 0x10B:  # PE32
        image_base = struct.unpack_from('<I', data, opt + 28)[0]
    elif magic == 0x20B:  # PE32+
        image_base = struct.unpack_from('<Q', data, opt + 24)[0]
    else:
        raise ValueError('bad magic')
    secs = []
    sec_off = opt + opt_size
    for i in range(num_sections):
        off = sec_off + i * 40
        name = data[off:off + 8].rstrip(b'\0').decode(errors='replace')
        vsize, vaddr, rsize, raddr = struct.unpack_from('<IIII', data, off + 8)
        flags = struct.unpack_from('<I', data, off + 36)[0]
        secs.append({
            'name': name, 'vaddr': vaddr, 'vsize': vsize,
            'raddr': raddr, 'rsize': rsize,
            'exec': bool(flags & 0x20000000),  # IMAGE_SCN_MEM_EXECUTE
        })
    return image_base, secs


def scan(data: bytes, secs, image_base: int, pat_bytes, mask):
    """Yield VA of every match in executable sections."""
    # Build a simple anchor from the longest run of non-wildcard bytes
    n = len(pat_bytes)
    # find longest solid run
    best_s = best_len = 0
    i = 0
    while i < n:
        if mask[i]:
            j = i
            while j < n and mask[j]:
                j += 1
            if j - i > best_len:
                best_s, best_len = i, j - i
            i = j
        else:
            i += 1
    if best_len == 0:
        raise ValueError('pattern is all wildcards')

    anchor = bytes(pat_bytes[best_s:best_s + best_len])
    for sec in secs:
        if not sec['exec']:
            continue
        blob = data[sec['raddr']:sec['raddr'] + sec['rsize']]
        start = 0
        while True:
            k = blob.find(anchor, start)
            if k == -1:
                break
            base = k - best_s
            if base >= 0 and base + n <= len(blob):
                ok = True
                for x in range(n):
                    if mask[x] and blob[base + x] != pat_bytes[x]:
                        ok = False
                        break
                if ok:
                    yield image_base + sec['vaddr'] + base
            start = k + 1


def parse_pattern(pat: str):
    tokens = pat.split()
    bts, mask = [], []
    for t in tokens:
        if '?' in t:
            bts.append(0)
            mask.append(False)
        else:
            bts.append(int(t, 16))
            mask.append(True)
    return bts, mask


IFACE_RE = re.compile(rb'\b([A-Z][A-Za-z0-9_]{3,40}0\d{2,3})\0')


def main():
    dll = Path(sys.argv[1])
    data = dll.read_bytes()
    image_base, secs = load_pe(data)

    if '--interfaces' in sys.argv:
        found = {}
        for sec in secs:
            blob = data[sec['raddr']:sec['raddr'] + sec['rsize']]
            for m in IFACE_RE.finditer(blob):
                s = m.group(1).decode()
                found.setdefault(s, set()).add(sec['name'])
        for s in sorted(found):
            print(f"{s:36s} [{','.join(sorted(found[s]))}]")
        return

    if '--json' in sys.argv:
        jf = Path(sys.argv[sys.argv.index('--json') + 1])
        pats = json.loads(jf.read_text())
        # optional module filter: --module client|engine|server|vguimatsurface|shaderapidx9
        mod_filter = None
        if '--module' in sys.argv:
            mod_filter = sys.argv[sys.argv.index('--module') + 1].lower()
        ok = fail = 0
        for i, p in enumerate(pats):
            if 'bytes' not in p:
                print(f"[UNPARSEABLE] {p['file']}:{p['line']} {p['pattern']!r}")
                fail += 1
                continue
            mods = p['module'].lower()
            if mod_filter and mod_filter not in mods:
                continue
            matches = list(scan(data, secs, image_base, p['bytes'], p['mask']))[:4]
            if matches:
                ok += 1
                print(f"[OK]   {p['file'].split('/')[-1]}:{p['line']:<5} -> " +
                      ', '.join(hex(x) for x in matches))
            else:
                fail += 1
                print(f"[FAIL] {p['file'].split('/')[-1]}:{p['line']:<5} {p['pattern']}")
        print(f"\n=== {ok} matched, {fail} failed ===")
        return

    # single pattern mode
    pat = sys.argv[2]
    bts, mask = parse_pattern(pat)
    for va in scan(data, secs, image_base, bts, mask):
        print(hex(va))


if __name__ == '__main__':
    main()
