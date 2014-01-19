//  Filters/Metering.cpp :: Audio buffer metrics collector
//  Copyright 2013 Keigen Shu

#include "Metering.h"

namespace awe {
namespace Filter {

AscMetering::AscMetering(Afloat freq, Afloat decay)
    : mFreq (freq)
    , mDecay(decay)
    , mPeak ({0.0f, 0.0f})
    , mRMS  ({0.0f, 0.0f})
    , mdPeak({0.0f, 0.0f})
    , mdRMS ({0.0f, 0.0f})
{

}

void AscMetering::doBuffer(AfBuffer &buffer)
{
    mPeak *= 0;
    mRMS  *= 0;

    /****/ if (buffer.getChannelCount() == 0) {
        return;
    } else if (buffer.getChannelCount() == 1) {
    } else {
        Asfloatf mSum({0.0f, 0.0f});
        for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
        {
            Afloat * f = buffer.getFrame(i);
            Asfloatf m ({std::abs(f[0]), std::abs(f[1])});

            mPeak[0] = std::max(mPeak[0], m[0]);
            mPeak[1] = std::max(mPeak[1], m[1]);

            mSum[0] += m[0] * m[0];
            mSum[1] += m[1] * m[1];
        }

        mSum /= buffer.getFrameCount();

        mRMS[0] = sqrt(mSum[0]);
        mRMS[1] = sqrt(mSum[1]);
    }

    mdRMS = mdRMS * mdRMS + mRMS * mRMS;
    mdRMS /= 2.0f;

    mdRMS[0] = sqrt(mdRMS[0]);
    mdRMS[1] = sqrt(mdRMS[1]);

    mdPeak[0] = std::max(mdPeak[0], mPeak[0]);
    mdPeak[1] = std::max(mdPeak[1], mPeak[1]);
}

}
}
