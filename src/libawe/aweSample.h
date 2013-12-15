//  aweSample.h :: Sound sample class
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_SAMPLE_H
#define AWE_SAMPLE_H

#include "aweLoop.h"
#include "aweSource.h"
#include "Filters/Mixer.h"

namespace awe {

class Asample : public Asource
{
private:
    typedef Filter::AscMixer AscMixer;

    AiBuffer*   mSource;
    Afloat      mSourcePeak;
    unsigned    mSampleRate;
    std::string mSampleName;

protected:
    AscMixer    mMixer;
    Aloop       mLoop;

public:
    /**
     * Default constructor
     */
    Asample(AiBuffer*   const &_source,
            Afloat      const &_peak,
            unsigned    const &_rate,
            std::string const &_name = "Unnamed sample",
            Aloop::Mode const &_loop = Aloop::Mode::__DEFAULT
           );

    Asample(Asample* _source);

    /**
     * Load from file constructor.
     * This function blocks execution and leaves source as nullptr if it fails to load the sample from file.
     */
    Asample(const std::string &file, const Aloop::Mode &_loop = Aloop::Mode::__DEFAULT);

    /**
     * Load from memory constructor.
     * This function blocks execution and leaves source as nullptr if it fails to load the sample from memory.
     */
    Asample(char* mptr, const size_t &size, const std::string &_name = "Unnamed sample", const Aloop::Mode &_loop = Aloop::Mode::__DEFAULT);

    virtual ~Asample() {}

    inline bool setSource (AiBuffer* const _source, const Afloat &_peak, const unsigned &_rate)
    {
        if (mSource == nullptr) {
            mSource     = _source;
            mSourcePeak = _peak;
            mSampleRate = _rate;
            mLoop.end   = _source->getFrameCount();
            return true;
        } else {
            return false;
        }
    }

    inline const AiBuffer * cgetSource() const { return mSource; }
    inline const AscMixer & cgetMixer () const { return mMixer; }
    inline const Aloop    & cgetLoop  () const { return mLoop; }

    inline       AiBuffer * getSource()       { return mSource; }
    inline const Afloat   & getPeak  () const { return mSourcePeak; }
    inline       AscMixer * getMixer ()       { return &mMixer; }
    inline       Aloop    * getLoop  ()       { return &mLoop; }
    inline const double   & getPos   () const { return mLoop.now; }
    inline       void       setPos   (double pos) { mLoop.now = pos; }

    inline const unsigned    & getSampleRate() const { return mSampleRate; }
    inline const std::string & getName      () const { return mSampleName; }
    inline void pause() { mLoop.paused = true; }
    inline void stop () { mLoop = Aloop(mLoop.begin, mLoop.end, mLoop.mode, true); }
    inline void play () { mLoop.paused = false; }
    inline void play (Afloat vol, Afloat pan, bool looping = false)
    {
        mMixer = AscMixer(vol, pan);
        mLoop.paused = false;
        mLoop.now    = mLoop.begin;
        mLoop.mode   = looping ? Aloop::Mode::REPEAT : Aloop::Mode::__DEFAULT;
    }

    /**
     * Skips the playing position of the sample to the supplied value.
     * If skip_silence is true, it will skip past the supplied value
     * until the current sample is not silent.
     *
     * @return number of silent samples skipped
     */
    size_t skip(const size_t &pos, const bool &skip_silence = false);

    // Asource pure function definition.
    virtual void make_active(void*) {
        stop();
        play();
    }

    virtual bool is_active() const { return !mLoop.paused; }
    /**
     * Render quality configuration:
     *  :: SKIP ::
     *      Returns without doing anything.
     *  :: MUTE ::
     *      Moves mLoop by the length of the output buffer and returns
     *      without doing anything to the output buffer.
     *
     *  :: FAST ::
     *      Mixes the sample source at it's raw form into the buffer.
     *      Volume, panning and sample magnitude reconstruction is not
     *      applied before mixing.
     *
     *  :: MEDIUM ::
     *      Mixes the sample source into the buffer.
     *      Volume, panning and sample magnitude reconstruction is
     *      applied before mixing.
     *
     *  :: BEST (DEFAULT) ::
     *      Mixes the sample source into the buffer, resampling the
     *      output if the source and target sample rates are not equal.
     *      Iterpolates if up-sampling. Decimates if down-sampling.
     */
    virtual void render(AfBuffer &buffer, const ArenderConfig &config);
    virtual void drop() { delete mSource; mSource = nullptr; }
};

}

#endif
