//  aweTrack.cpp :: Sound mixing track
//  Copyright 2012 - 2013 Keigen Shu

#include "aweTrack.h"

namespace awe {

void Atrack::fpull(Asource* src)
{
    if (src->is_active() == true)
        src->render(mPbuffer, mPconfig);
}

void Atrack::fpull()
{
    for(Asource* src: mPsources)
        fpull(src);
}

void Atrack::fflip()
{
    mObuffer.reset();
    mObuffer.swap(mPbuffer);
}

void Atrack::ffilter()
{
    mOfilter.doBuffer(mObuffer);
}


Atrack::Atrack(
    const size_t &sample_rate,
    const size_t &frames,
    const std::string &name
)   : mName   (name)
    , mPconfig(sample_rate, frames)
    , mPbuffer(2, frames)
    , mObuffer(2, frames)
    , mqActive(true)
{ }

void Atrack::render(AfBuffer &targetBuffer, const ArenderConfig &targetConfig)
{
    std::lock(mPmutex, mOmutex);

    size_t a = 0, p = targetConfig.targetFrameOffset;
    size_t const  q = targetConfig.targetFrameOffset + mPconfig.targetFrameCount;

    MutexLockGuard o_lock(mOmutex, std::adopt_lock);
    {
        // Unlock pool mutex immediately after mixing.
        MutexLockGuard p_lock(mPmutex, std::adopt_lock);
        fpull();
        fflip();
    }

    ffilter();

    AfBuffer::const_pointer src = mObuffer.data();
    AfBuffer::      pointer dst = targetBuffer.data();

    while(p < q)
    {
        dst[p*2  ] += src[a*2  ];
        dst[p*2+1] += src[a*2+1];

        p += 1;
        a += 1;
    }
}

}
