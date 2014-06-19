/* Filters/3BEQ.h :: 3-band equalizer
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
#ifndef AWE_FILTER_3BAND_EQ_H
#define AWE_FILTER_3BAND_EQ_H

#include "../aweFilter.h"
#include "IIR.h"
#include <cassert>

namespace awe {
namespace Filter {

template< const Achan Channels >
class TBEQ : public Afilter< Channels >
{
private:
    double mSF, mLF, mHF; //  Current sampling, low-pass and high-pass frequencies

    IIR::IIR< Channels > mLP;
    IIR::IIR< Channels > mHP;

    double mLG, mMG, mHG;

public:
    TBEQ(
        double mixfreq,
        double lo_freq = 880.0,
        double hi_freq = 5000.0,
        double lo_gain = 1.0,
        double mi_gain = 1.0,
        double hi_gain = 1.0
    )   : mSF(mixfreq)
        , mLF(lo_freq)
        , mHF(hi_freq)
        , mLP(IIR::newLPF(mSF, mLF))
        , mHP(IIR::newHPF(mSF, mHF))
        , mLG(lo_gain)
        , mMG(mi_gain)
        , mHG(hi_gain)
    { }


    inline void reset_state()
    {
        mLP.reset();
        mHP.reset();
    }

    inline void get_freq(double &lo_freq, double &hi_freq) const
    {
        lo_freq = mLF;
        hi_freq = mHF;
    }

    inline void set_freq(double mixfreq)
    {
        mSF = mixfreq;
        mLP = IIR::newLPF(mSF, mLF);
        mHP = IIR::newHPF(mSF, mHF);
    }

    inline void set_freq(double lo_freq, double hi_freq)
    {
        mLF = lo_freq;
        mHF = hi_freq;
        set_freq(mSF);
    }

    inline void get_gain(double &lo_gain, double &mi_gain, double &hi_gain) const
    {
        lo_gain = mLG;
        mi_gain = mMG;
        hi_gain = mHG;
    }

    inline void set_gain(double lo_gain, double mi_gain, double hi_gain)
    {
        mLG = lo_gain;
        mMG = mi_gain;
        mHG = hi_gain;
    }

    inline void doBuffer(AfBuffer &buffer)
    {
        assert(buffer.getChannelCount() == Channels);

        for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
        {
            Afloat* f = buffer.getFrame(i);
            for(Achan c = 0; c < Channels; c += 1)
            {
                double L = f[c];
                double H = f[c];

                mLP.process(c, L);
                mHP.process(c, H);

                double M = f[c] - (L + H);

                L *= mLG;
                M *= mMG;
                H *= mHG;

                f[c] = static_cast<Afloat>(L + M + H);
            }

        }
    }

};

}
}

#endif
