//  Filters/Maximizer.h :: Basic mixer filter
//  Copyright 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_FILTER_MAXIMIZER_H
#define AWE_FILTER_MAXIMIZER_H

#include "../aweFilter.h"

namespace awe {
namespace Filter {

//! Calculates differential for `a` to `b` for time period `t` from data sampled
//! at a sampling rate of `f`. Here it is used to calculate the decay slope.
inline float calc_diff(float a, float b, float t, float f) {
    return (a - b) / (t * f);
}

/** The libawe Maximizer.
 *  A simple hard-limiter that boosts and limits audio signals.
 */
template< const Achan Channels >
class Maximizer : public Afilter< Channels >
{
private:
    unsigned    mFrameRate;     //!< Frame rate

    Afloat      mBoost;         //!< Pre-limiter boost
    Afloat      mThreshold;     //!< Limiter threshold
    Afloat      mSlowRelease;   //!< Slow-release time in milliseconds
    Afloat      mPeakRelease;   //!< Peak-release time in milliseconds
    Afloat      mCeiling;       //!< Post-limiter ceiling

    Afloat      mDecayRate;     //!< Current release rate
    Afloat      mGain;          //!< Current limiter level

    ////    Metering attributes    ////
    Afloat      mPeakSample;    //<! Peak sample on last update

public:
    //! Default constructor.
    Maximizer(
        unsigned frame_rate,
        Afloat boost        = awe::from_dBFS( 0.0f),
        Afloat threshold    = awe::from_dBFS(-0.1f),
        Afloat slow_release = 1.0f,
        Afloat peak_release = 200.0f,
        Afloat ceiling      = awe::from_dBFS( 0.0f)
    )   : mFrameRate    (frame_rate)
        , mBoost        (boost)
        , mThreshold    (threshold)
        , mSlowRelease  (slow_release)
        , mPeakRelease  (peak_release)
        , mCeiling      (ceiling)
        , mDecayRate    (0.0f)
        , mGain         (1.0f)
        , mPeakSample   (0.0f)
    { }

    //! Resets the limiter state of the maximizer.
    inline void reset_state() override {
        mDecayRate  = 0.0f;
        mGain       = 1.0f;
        mPeakSample = 0.0f;
    }

    inline void setBoost        (Afloat const &value) { mBoost       = value; }
    inline void setThreshold    (Afloat const &value) { mThreshold   = value; reset_state(); }
    inline void setSlowRelease  (Afloat const &value) { mSlowRelease = value; reset_state(); }
    inline void setPeakRelease  (Afloat const &value) { mPeakRelease = value; reset_state(); }
    inline void setCeiling      (Afloat const &value) { mCeiling     = value; }

    inline Afloat getBoost      () const { return mBoost; }
    inline Afloat getThreshold  () const { return mThreshold; }
    inline Afloat getSlowRelease() const { return mSlowRelease; }
    inline Afloat getPeakRelease() const { return mPeakRelease; }
    inline Afloat getCeiling    () const { return mCeiling; }

    inline Afloat getCurrentGain() const { return mGain; }
    inline Afloat getPeakSample () const { return mPeakSample; }

    void doBuffer(AfBuffer &buffer) override
    {
        assert(buffer.getChannelCount() == Channels);

        mPeakSample = 0.0f;

        for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
        {
            Afloat* frame = buffer.getFrame(i);
            Afloat  framePeak = 0.0f;

            //  Get peak value in frame.
            for(Achan c = 0; c < Channels; c += 1) {
                frame[c] *= mBoost;
                framePeak = std::max(framePeak, std::abs(frame[c]));
            }

            mPeakSample = std::max(mPeakSample, framePeak);

            //  Current peak is over threshold
            if (framePeak > mThreshold) {
                Afloat newGain = framePeak / mThreshold;

                //  Reset decay rate if higher limiting
                if (newGain > mGain) {
                    mGain = newGain;
                    mDecayRate = ( (framePeak > 1.0f)
                            ? calc_diff(framePeak,       1.0f, mPeakRelease / 1000.0f, mFrameRate)
                            : calc_diff(framePeak, mThreshold, mSlowRelease / 1000.0f, mFrameRate)
                            );
                }
            }

            for(Achan c = 0; c < Channels; c += 1)
                frame[c] *= mCeiling / mGain;

            if (mGain > 1.0f / mThreshold)
            {
                mGain = mGain - mDecayRate;
                //  Switch to slow release if limiter gain has reached threshold.
                if (mGain < 1.0f / mThreshold)
                    mDecayRate = calc_diff(1.0f, mThreshold, mSlowRelease / 1000.0f, mFrameRate);

            } else if (mGain > 1.0f) {
                mGain = mGain - mDecayRate;
            } else {
                //  Limiter gain is below threshold.
                mGain = 1.0f;
            }
        }
    }
};

}
}
#endif
