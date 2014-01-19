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

namespace awe {
namespace Filter {

Asc3BEQ::Asc3BEQ (
    double mixfreq,
    double lo_freq,
    double hi_freq,
    double lo_gain,
    double mi_gain,
    double hi_gain
)   : mSF(mixfreq)
    , mLF(lo_freq)
    , mHF(hi_freq)
    , mLP(AscIIR::newLPF(mSF, mLF))
    , mHP(AscIIR::newHPF(mSF, mHF))
    , mLG(lo_gain)
    , mMG(mi_gain)
    , mHG(hi_gain)
{}

Afloat Asc3BEQ::ffdoL(Afloat const &l)
{
    double L = l;
    double H = l;

    mLP.processL(L);
    mHP.processL(H);

    double M = l - (L + H);

    L *= mLG;
    M *= mMG;
    H *= mHG;

    // Return result
    return static_cast<Afloat>(L + M + H);
}

Afloat Asc3BEQ::ffdoR(Afloat const &r)
{
    double L = r;
    double H = r;

    mLP.processR(L);
    mHP.processR(H);

    double M = r - (L + H);

    L *= mLG;
    M *= mMG;
    H *= mHG;

    // Return result
    return static_cast<Afloat>(L + M + H);
}

}
}
