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

class Asc3BEQ : public AscFilter
{
private:
    double mSF, mLF, mHF; //  Current sampling, low-pass and high-pass frequencies

    AscIIR mLP;
    AscIIR mHP;

    double mLG, mMG, mHG;

public:
    Asc3BEQ(
        double mixfreq,
        double lo_freq = 880.0,
        double hi_freq = 5000.0,
        double lo_gain = 1.0,
        double mi_gain = 1.0,
        double hi_gain = 1.0
    );

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
        mLP = AscIIR::newLPF(mSF, mLF);
        mHP = AscIIR::newHPF(mSF, mHF);
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

    inline void doBuffer(AfBuffer &buffer) {
        /****/ if (buffer.getChannelCount() == 0) {
            return;
        } else if (buffer.getChannelCount() == 1) {
            std::for_each(
                    buffer.begin(), buffer.end(),
                    [this](Afloat &value) {
                        value = this->ffdoL(value) + this->ffdoR(value);
                        value = value / 2.0f;
                    });
        } else {
            for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
            {
                Afloat* f = buffer.getFrame(i);
                f[0] = ffdoL(f[0]);
                f[1] = ffdoR(f[1]);
            }
        }
    }

    Afloat ffdoL(Afloat const &l);
    Afloat ffdoR(Afloat const &r);

};

}
}

#endif
