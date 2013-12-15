#!/usr/bin/env python3

import sys

"""
Judgement table calculator
Script by Keigen Shu
"""


# Convert beats per minute to milliseconds per tick
def bpm_to_mspt(bpm, tpb):
    return 60000.0 / (float(tpb) * float(bpm))


# Input integer real duration (in ms) to float tick duration (in ticks)
def iRD_to_fTD(arr, mspt):
    n = []
    for i in arr:
        n.append(float(i) / mspt)

    return n


# Float tick duration to rounded integer tick duration
def fTD_to_iTD(arr):
    n = []
    for i in arr:
        n.append(int(round(i)))

    return n


# Rounded integer tick duration to adjusted integer tick duration
def iTD_to_jTD(arr, fac):
    n = []
    for i in arr:
        n.append(i + fac - (i % fac))

    return n


# Pretty print
def to_str(index, fTD, iTD, jTD, mst):
    return '%.3f (%d) >> %d ticks  ||  %.3f >> %.3f ms' % (
        fTD[index], iTD[index], jTD[index],
        fTD[index] * mst, jTD[index] * mst
    )


def main():
    if len(sys.argv) < 7:
        print('Usage: %s <TPB> <Pd> <Cd> <Gd> <Bd> [wrap]' % sys.argv[0])
        sys.exit('Invalid number of arguments')

    tpb = int(sys.argv[1])
    fac = int(sys.argv[6]) if len(sys.argv) == 7 else 2
    iRD = [
        int(sys.argv[2]),
        int(sys.argv[3]),
        int(sys.argv[4]),
        int(sys.argv[5])
    ]

    for bpm in [48, 96, 128, 140, 150, 160, 180, 240, 300, 360, 480, 960]:
        mspt = bpm_to_mspt(bpm, tpb)

        print()
        print(
            'beat/minute = %f  -->  ms/tick = %f  ::  ceiling factor = %d'
            % (bpm, mspt, fac)
        )

        fTD = iRD_to_fTD(iRD, mspt)

        iTD = fTD_to_iTD(fTD)

        jTD = iTD_to_jTD(iTD, fac)

        print('PFCT \t%s' % to_str(0, fTD, iTD, jTD, mspt))
        print('COOL \t%s' % to_str(1, fTD, iTD, jTD, mspt))
        print('GOOD \t%s' % to_str(2, fTD, iTD, jTD, mspt))
        print('BAAD \t%s' % to_str(3, fTD, iTD, jTD, mspt))


if __name__ == '__main__':
    main()
