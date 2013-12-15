//  aweEngine.h :: Core sound engine
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_ENGINE_H
#define AWE_ENGINE_H

#include "aweTrack.h"
#include "awePortAudio.h"

namespace awe {

/** Master audio output interface
 * This class manages the audio output to an audio output library.
 */
class AEngine
{
protected:
    APortAudio  mOutputDevice;
    Atrack      mMasterTrack;

public:
    AEngine(size_t output_sample_rate = 48000,
            size_t output_frame_rate = 4096,
            APortAudio::HostAPIType device_type = APortAudio::HostAPIType::Default
            ) :
        mOutputDevice(),
        mMasterTrack (output_sample_rate, output_frame_rate, "Output to Device")
    {
        if (mOutputDevice.init(output_sample_rate, output_frame_rate, device_type) == false)
            throw std::runtime_error("libawe [exception] Could not initialize output device.");
    }

    virtual ~AEngine() { mOutputDevice.shutdown(); }

    inline Atrack& getMasterTrack() { return mMasterTrack; }

    /* TODO Add some sort of multi-threading support. */
    virtual bool update()
    {
        if (mOutputDevice.getFIFOBuffer().size() < mMasterTrack.getOutput().getSampleCount())
        {
            // Process stuff
            mMasterTrack.pull();
            mMasterTrack.flip();

            // Push to output device buffer
            mOutputDevice.getFIFOBuffer_mutex().lock();
            mMasterTrack.push(mOutputDevice.getFIFOBuffer());
            mOutputDevice.getFIFOBuffer_mutex().unlock();

            return true;
        } else {
            return false;
        }
    }
};

}

#endif
