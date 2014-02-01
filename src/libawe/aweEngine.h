//  aweEngine.h :: Core sound engine
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_ENGINE_H
#define AWE_ENGINE_H

#include "aweTrack.h"
#include "awePortAudio.h"

namespace awe {

/**
 * Master audio output interface.
 *
 * This class manages the output of audio from libawe into the sound
 * device via PortAudio.
 */
class AEngine
{
protected:
    APortAudio  mOutputDevice;  //! PortAudio output device wrapper
    Atrack      mMasterTrack;   //! Master output track

public:
    AEngine(
        size_t sampling_rate = 48000,
        size_t op_frame_rate = 4096,
        APortAudio::HostAPIType device_type = APortAudio::HostAPIType::Default
    ) : mOutputDevice(),
        mMasterTrack (sampling_rate, op_frame_rate, "Output to Device")
    {
        if (mOutputDevice.init(sampling_rate, op_frame_rate, device_type) == false)
            throw std::runtime_error("libawe [exception] Could not initialize output device.");
    }

    virtual ~AEngine() { mOutputDevice.shutdown(); }

    inline Atrack& getMasterTrack() { return mMasterTrack; }

    /**
     * Pulls audio mix from master track and pushes them into the
     * output device.
     *
     * This operation is done only if the buffer in the output device
     * does not have enough data for when the system demands them.
     *
     * This call may take a very long time to complete depending on the
     * amount of work required to pull audio data into the master track
     * and then mix them.
     *
     * @return False if the output device buffer has sufficient data
     *         for the next time the system requests for them.
     */
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
