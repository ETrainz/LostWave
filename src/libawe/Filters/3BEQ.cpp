/* Filters/3BEQ.cpp :: 3-band equalizer
 * Taken from http://www.musicdsp.org/archive.php?classid=3#236
 *
 * Copyright 2006 Neil C. / Etanza Systems
 *
 * This file is hereby placed under the public domain for all purposes,
 * including use in commercial applications.
 *
 * The author assumes NO RESPONSIBILITY for any of the problems caused
 * by the use of this software.
 */
#include "3BEQ.h"
#include <cmath>

namespace awe {
namespace Filter {

Asc3BEQ::Asc3BEQ (
        double mixfreq,
        double lo_freq,
        double hi_freq,
        double lo_gain,
        double mi_gain,
        double hi_gain
        ) :
    lf(2.0 * sin(M_PI * lo_freq / mixfreq)),
    hf(2.0 * sin(M_PI * hi_freq / mixfreq)),
    lg(lo_gain),
    mg(mi_gain),
    hg(hi_gain),
    llp{0.0, 0.0, 0.0, 0.0},
    lhp{0.0, 0.0, 0.0, 0.0},
    rlp{0.0, 0.0, 0.0, 0.0},
    rhp{0.0, 0.0, 0.0, 0.0},
    la{0.0, 0.0, 0.0},
    ra{0.0, 0.0, 0.0}
{}

Afloat Asc3BEQ::ffdoL(Afloat const &l)
{
    double L, M, H; // low / mid / high - sample values

    // Low-pass
    llp[0] += (lf * (l - llp[0])) + d;
    llp[1] += (lf * (llp[0] - llp[1]));
    llp[2] += (lf * (llp[1] - llp[2]));
    llp[3] += (lf * (llp[2] - llp[3]));

    L = llp[3];

    // High-pass
    lhp[0] += (hf * (l - lhp[0])) + d;
    lhp[1] += (hf * (lhp[0] - lhp[1]));
    lhp[2] += (hf * (lhp[1] - lhp[2]));
    lhp[3] += (hf * (lhp[2] - lhp[3]));

    H = la[2] - lhp[3];

    // Calculate mid-range
    M = la[2] - (H + L);

    // Apply gain
    L *= lg;
    M *= mg;
    H *= hg;

    // Process accumulator
    la[2] = la[1];
    la[1] = la[0];
    la[0] = l;

    // Return result
    return static_cast<Afloat>(L + M + H);
}

Afloat Asc3BEQ::ffdoR(Afloat const &r) {
    double L, M, H; // low / mid / high - sample values

    // Low-pass
    rlp[0] += (lf * (r - rlp[0])) + d;
    rlp[1] += (lf * (rlp[0] - rlp[1]));
    rlp[2] += (lf * (rlp[1] - rlp[2]));
    rlp[3] += (lf * (rlp[2] - rlp[3]));

    L = rlp[3];

    // High-pass
    rhp[0] += (hf * (r - rhp[0])) + d;
    rhp[1] += (hf * (rhp[0] - rhp[1]));
    rhp[2] += (hf * (rhp[1] - rhp[2]));
    rhp[3] += (hf * (rhp[2] - rhp[3]));

    H = ra[2] - rhp[3];

    // Calculate mid-range
    M = ra[2] - (H + L);

    // Apply gain
    L *= lg;
    M *= mg;
    H *= hg;

    // Process accumulator
    ra[2] = ra[1];
    ra[1] = ra[0];
    ra[0] = r;

    // Return result
    return static_cast<Afloat>(L + M + H);
}

}
}
