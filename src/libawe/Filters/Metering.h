//  Filters/Metering.h :: Audio buffer metrics collector
//  Copyright 2013 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_FILTER_METERING_H
#define AWE_FILTER_METERING_H

#include "../aweFilter.h"

namespace awe {
namespace Filter {

/*! Stereo channel metering filter.
 *
 *  This filter measures the peak and root mean square magnitudes of the
 *  audio signals that passes through. The output values are normalized
 *  linear scale floating points (not the decibel scale).
 */
class AscMetering : public AscFilter
{
private:
    Afloat      mFreq;  //!< Source buffer frequency
    Afloat      mDecay; //!< Decay period

    // Per buffer parameters
    Asfloatf    mPeak;  //!< Buffer peak
    Asfloatf    mRMS;   //!< Buffer root mean square

    // Decaying parameters
    Asintf      mdOCI;  //!< Overclip indicator
    Asfloatf    mdRMS;

public:
    AscMetering(Afloat freq, Afloat decay);

    Asfloatf const &getPeak     () const { return mPeak; }
    Asfloatf const &getRMS      () const { return mRMS; }
    Asintf   const &getOCI      () const { return mdOCI; }
    Asfloatf const &getAvgRMS   () const { return mdRMS; }

    void reset_state() override {
        mPeak  *= 0;
        mRMS   *= 0;
        mdOCI  *= 0;
        mdRMS  *= 0;
    }
    void doBuffer(AfBuffer &buffer) override;
};
}
}
#endif
