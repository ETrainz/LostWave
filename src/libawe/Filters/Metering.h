//  Filters/Metering.h :: Audio buffer metrics collector
//  Copyright 2013 Keigen Shu

#ifndef AWE_FILTER_INFO_H
#define AWE_FILTER_INFO_H

#include "../aweFilter.h"

namespace awe {
namespace Filter {

class AscMetering : public AscFilter
{
private:
    Afloat      mFreq;  // Source buffer frequency
    Afloat      mDecay; // Decay period

    // Per buffer parameters
    Asfloatf    mPeak; // Buffer peak
    Asfloatf    mRMS;  // Buffer root mean square

    // Decaying parameters
    Asfloatf    mdPeak;
    Asfloatf    mdRMS;

public:
    AscMetering(Afloat freq, Afloat decay);

    Asfloatf const &getPeak     () const { return mPeak; }
    Asfloatf const &getRMS      () const { return mRMS; }
    Asfloatf const &getAvgPeak  () const { return mdPeak; }
    Asfloatf const &getAvgRMS   () const { return mdRMS; }

    void reset_state() {
        mPeak  *= 0;
        mRMS   *= 0;
        mdPeak *= 0;
        mdRMS  *= 0;
    }
    void doBuffer(AfBuffer &buffer);
};
}
}
#endif
