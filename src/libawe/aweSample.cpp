//  aweSample.cpp :: Sound sample class
//  Copyright 2012 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#include "aweSample.h"

namespace awe {

Asample::Asample(
    AiBuffer*   const &_source,
    Afloat      const &_peak,
    unsigned    const &_rate,
    std::string const &_name,
    Aloop::Mode const &_loop
)   : Asource()
    , mSource(_source)
    , mSourcePeak(_peak)
    , mSampleRate(_rate)
    , mSampleName(_name)
    , mLoop(0, 0, _source == nullptr ? 0 : _source->getFrameCount(), _loop)
{ }

Asample::Asample(Asample* _source)
    : Asource()
    , mSource(_source->getSource())
    , mSourcePeak(_source->getPeak())
    , mSampleRate(_source->getSampleRate())
    , mSampleName(_source->getName())
    , mMixer(_source->cgetMixer())
    , mLoop(_source->cgetLoop())
{ }

void Asample::render(AfBuffer &buffer, const ArenderConfig &config)
{
    if (mSource == nullptr) return;

    double const move_rate = static_cast<double>(mSampleRate) / static_cast<double>(config.targetSampleRate);

    switch(config.quality)
    {
        case ArenderConfig::Quality::MUTE:
            mLoop += config.targetFrameCount * move_rate;
        case ArenderConfig::Quality::SKIP:
            return;

        case ArenderConfig::Quality::FAST:
            /****/ if (mSource->getChannelCount() >= 2)
            {
                for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
                {
                    const unsigned long z = mLoop.unow() * mSource->getChannelCount();
                    buffer.data()[i*2  ] += mSource->get0Sample(z  );
                    buffer.data()[i*2+1] += mSource->get0Sample(z+1);
                    if (mLoop += move_rate) return;
                }
            } else if (mSource->getChannelCount() == 1) {
                for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
                {
                    const unsigned long z = mLoop.unow();
                    buffer.data()[i*2  ] += mSource->get0Sample(z);
                    buffer.data()[i*2+1] += mSource->get0Sample(z);
                    if (mLoop += move_rate) return;
                }
            }
            break;
        default:
            break;
    }
    if (config.quality == ArenderConfig::Quality::MEDIUM
        || mSampleRate == config.targetSampleRate
    ) {
        /****/ if (mSource->getChannelCount() >= 2)
        {
            for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
            {
                const unsigned long z = mLoop.unow() * mSource->getChannelCount();
                buffer.data()[i*2  ] += mMixer.ifdoL(mSource->get0Sample(z  )) * mSourcePeak;
                buffer.data()[i*2+1] += mMixer.ifdoR(mSource->get0Sample(z+1)) * mSourcePeak;
                if (mLoop += move_rate) return;
            }
        } else if (mSource->getChannelCount() == 1) {
            for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
            {
                const unsigned long z = mLoop.unow();
                buffer.data()[i*2  ] += mMixer.ifdoL(mSource->get0Sample(z  )) * mSourcePeak;
                buffer.data()[i*2+1] += mMixer.ifdoR(mSource->get0Sample(z  )) * mSourcePeak;
                if (mLoop += move_rate) return;
            }
        }
    } else { // config.quality == BEST || DEFAULT
#if 0
        if (move_rate > 1.0) // Down-sampling
        {

        } else { // Up-sampling

        }

        if (move_rate > 1.0)
            fprintf(stderr, "libawe [debug] Proper down-sampling has not been implemented yet.");
#endif

        /****/ if (mSource->getChannelCount() >= 2)
        {
            for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
            {
                const unsigned long z = mLoop.unow() * mSource->getChannelCount();
                buffer.data()[i*2  ] += mMixer.ifdoL(mSource->getiSample(z  )) * mSourcePeak;
                buffer.data()[i*2+1] += mMixer.ifdoR(mSource->getiSample(z+1)) * mSourcePeak;
                if (mLoop += move_rate) return;
            }
        } else if (mSource->getChannelCount() == 1) {
            for(size_t i = config.targetFrameOffset; i < config.targetFrameOffset + config.targetFrameCount; i++)
            {
                const unsigned long z = mLoop.unow();
                buffer.data()[i*2  ] += mMixer.ifdoL(mSource->getiSample(z  )) * mSourcePeak;
                buffer.data()[i*2+1] += mMixer.ifdoR(mSource->getiSample(z  )) * mSourcePeak;
                if (mLoop += move_rate) return;
            }
        }
    }
}

size_t Asample::skip(const size_t &pos, const bool &skip_silence)
{
    size_t i = pos > mLoop.uend() ? mLoop.uend() : pos;
    size_t j = 0;

    if (skip_silence == true)
        for(; mSource->getSample(i) != 0 && i < mLoop.uend(); ++j)
            ++ i;

    mLoop.now = i / mSource->getChannelCount();

    return j;
}

}
