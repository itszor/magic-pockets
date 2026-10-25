#!/usr/bin/env python3
"""The two programs, unpacked by Loader's own decompressor.

    unpack.py           game/!Pockets/Intro, Pockets -> unpacked/intro, pockets
                        (the images Loader runs, at &8000)

!Run runs Loader (an absolute at &707E0). It reads Demo$Mode: with bit 1
set (!Run sets 2) it loads Intro at &8000, otherwise Pockets; both are Rob
Northern's ProPack (RNC method 1: "RNC\\1", the unpacked and packed lengths,
big-endian). &70950 unpacks the file in place (r0 the packed data, r1 where
it goes: both &8000; it moves the packed data up out of the way first), and
Loader then enters Intro at &8088 or Pockets at &8614. Here that routine
runs under SuperSpider's tools/arm2.py.
"""
import os
import struct
import sys

SS = os.environ.get('SUPERSPIDER', os.path.expanduser('~/stuff/superspider'))
sys.path.insert(0, os.path.join(SS, 'tools'))
import arm2  # noqa: E402

TOP = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
APP = os.path.join(TOP, 'game', '!Pockets')
OUT = os.path.join(TOP, 'unpacked')
LOADER, IMAGE, UNPACK = 0x707E0, 0x8000, 0x70950
STACK, DONE = 0x1F0000, 0x3FFFFF0


def find(d, name):
    """a RISC OS file in a host directory: any case, any ,load-exec suffix"""
    for f in os.listdir(d):
        if f.split(',')[0].lower() == name.lower():
            return os.path.join(d, f)
    raise FileNotFoundError(os.path.join(d, name))


def unpack(loader, packed):
    if packed[:4] != b'RNC\1':
        sys.exit('unpack: not an RNC method 1 file')
    n = struct.unpack_from('>I', packed, 4)[0]
    m = arm2.ARM(memsize=0x4000000)
    m.mem[LOADER:LOADER + len(loader)] = loader
    m.mem[IMAGE:IMAGE + len(packed)] = packed
    m.r[0], m.r[1] = IMAGE, IMAGE
    m.r[13], m.r[14] = STACK, DONE                     # (it returns with movs pc,lr)
    m.r[15] = UNPACK
    while m.r[15] != DONE:
        m.step()
        if m.steps > 200_000_000:
            raise RuntimeError('unpack: no end, pc=&%X' % m.r[15])
    if m.r[13] != STACK:
        sys.exit('unpack: the decompressor failed (a damaged dump?)')
    return bytes(m.mem[IMAGE:IMAGE + n])


def main():
    os.makedirs(OUT, exist_ok=True)
    loader = open(find(APP, 'Loader'), 'rb').read()
    for name in ('Intro', 'Pockets'):
        data = unpack(loader, open(find(APP, name), 'rb').read())
        open(os.path.join(OUT, name.lower()), 'wb').write(data)
        print('%s: %d bytes at &%X' % (name.lower(), len(data), IMAGE))


if __name__ == '__main__':
    main()
