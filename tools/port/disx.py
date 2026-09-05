#!/usr/bin/env python3
"""Disassembly helper for the 2016 CS:GO binaries.

Usage:
  python dis.py <dll> <rva-hex> [bytes]      # disasm N bytes at RVA
  python dis.py <dll> findstr "<string>"     # RVA(s) of ascii/utf16 string
  python dis.py <dll> xrefs <rva-hex>        # find push <imm32>/mov references to RVA
"""
import sys
import struct
from pathlib import Path
import capstone

DLLDIR = Path('/home/user/port/dlls/dlls')

_cache = {}

def load(dll):
    if dll in _cache:
        return _cache[dll]
    data = (DLLDIR / dll).read_bytes()
    e = struct.unpack_from('<I', data, 0x3C)[0]
    machine, nsec, _, _, _, opt_size, _ = struct.unpack_from('<HHIIIHH', data, e + 4)
    opt = e + 24
    image_base = struct.unpack_from('<I', data, opt + 28)[0]
    secs = []
    sec_off = opt + opt_size
    for i in range(nsec):
        off = sec_off + i * 40
        name = data[off:off + 8].rstrip(b'\0').decode()
        vsize, vaddr, rsize, raddr = struct.unpack_from('<IIII', data, off + 8)
        flags = struct.unpack_from('<I', data, off + 36)[0]
        secs.append(dict(name=name, vaddr=vaddr, vsize=vsize, raddr=raddr,
                         rsize=rsize, exec=bool(flags & 0x20000000)))
    _cache[dll] = (data, image_base, secs)
    return _cache[dll]

def rva_to_off(secs, rva):
    for s in secs:
        if s['vaddr'] <= rva < s['vaddr'] + max(s['vsize'], s['rsize']):
            return s['raddr'] + (rva - s['vaddr'])
    return None

def dis(dll, rva, n=120):
    data, ib, secs = load(dll)
    off = rva_to_off(secs, rva)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = False
    code = data[off:off + n]
    for ins in md.disasm(code, ib + rva):
        print(f"{ins.address - ib:#010x}  {ins.bytes.hex():<20} {ins.mnemonic} {ins.op_str}")

def find_str(dll, s):
    data, ib, secs = load(dll)
    res = []
    for enc in ('ascii', 'utf16'):
        needle = s.encode() if enc == 'ascii' else s.encode('utf-16-le')
        start = 0
        while True:
            k = data.find(needle + (b'\0' if enc == 'ascii' else b'\0\0'), start)
            if k == -1:
                break
            # file offset -> rva
            for sec in secs:
                if sec['raddr'] <= k < sec['raddr'] + sec['rsize']:
                    rva = sec['vaddr'] + (k - sec['raddr'])
                    res.append((rva, enc, sec['name']))
            start = k + 1
    for rva, enc, sec in res:
        print(f"{rva:#010x} [{enc}/{sec}]")
    return res

def xrefs(dll, rva):
    """Find imm32 references (push/mov/lea/cmp) to given RVA."""
    data, ib, secs = load(dll)
    target = struct.pack('<I', ib + rva)
    out = []
    for sec in secs:
        if not sec['exec']:
            continue
        blob = data[sec['raddr']:sec['raddr'] + sec['rsize']]
        start = 0
        while True:
            k = blob.find(target, start)
            if k == -1:
                break
            out.append(sec['vaddr'] + k)
            start = k + 1
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for r in out[:60]:
        # disasm a window before to see the instruction
        print(f"--- ref at {r:#010x} ---")
        for back in (1, 2, 3, 5, 6, 7):
            off = rva_to_off(secs, r - back)
            if off is None:
                continue
            code = data[off:off + 24]
            insns = list(md.disasm(code, ib + r - back))
            if insns and insns[0].address + len(insns[0].bytes) == ib + r:
                for ins in insns[:4]:
                    print(f"  {ins.address - ib:#010x}  {ins.bytes.hex():<16} {ins.mnemonic} {ins.op_str}")
                break
    return out

if __name__ == '__main__':
    dll = sys.argv[1]
    cmd = sys.argv[2]
    if cmd == 'findstr':
        find_str(dll, sys.argv[3])
    elif cmd == 'xrefs':
        xrefs(dll, int(sys.argv[3], 16))
    else:
        rva = int(cmd, 16)
        n = int(sys.argv[3]) if len(sys.argv) > 3 else 120
        dis(dll, rva, n)
