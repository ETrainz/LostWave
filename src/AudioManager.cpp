#include "AudioManager.hpp"
#include "libawe/Filters/3BEQ.h"
#include "libawe/Filters/Mixer.h"
#include <chrono>
#include <list>

AudioManager::AudioManager(size_t frame_count, size_t sample_rate)
    : awe::AEngine(sample_rate, frame_count)
    , mUpdateCount(0)
    // , mRunning(ATOMIC_FLAG_INIT)
{
    mTrackMap.insert({
            { 0, new Track(sample_rate, frame_count, "Autoplay") },
            { 1, new Track(sample_rate, frame_count, "Player 1") },
            { 2, new Track(sample_rate, frame_count, "Player 2") }
            });
    /*
    mTrackMap[0]->getRack().attach_filter(new awe::Filter::AscMixer(1.0, 0.0));
    mTrackMap[1]->getRack().attach_filter(new awe::Filter::AscMixer(1.0, 0.0));
    mTrackMap[2]->getRack().attach_filter(new awe::Filter::AscMixer(1.0, 0.0));

    mTrackMap[0]->getRack().attach_filter(new awe::Filter::Asc3BEQ(sample_rate / 2, 880.0, 5000.0, 1.0, 0.7, 0.8));
    mTrackMap[1]->getRack().attach_filter(new awe::Filter::Asc3BEQ(sample_rate / 2, 880.0, 5000.0, 1.0, 0.7, 0.8));
    mTrackMap[2]->getRack().attach_filter(new awe::Filter::Asc3BEQ(sample_rate / 2, 880.0, 5000.0, 1.0, 0.7, 0.8));
    */

    mMasterTrack.attach_source(mTrackMap[0]);
    mMasterTrack.attach_source(mTrackMap[1]);
    mMasterTrack.attach_source(mTrackMap[2]);

    mRunning.test_and_set();

    mThreads.push_back(
        new std::thread( [this] () {
            while(mRunning.test_and_set())
            {
                if (this->update() == false)
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                else
                    mUpdateCount += 1;
            }
            mRunning.clear();
        })
    );
}

AudioManager::~AudioManager()
{
    mRunning.clear();
    while(mThreads.empty() == false)
    {
        auto it = mThreads.begin();
        (*it)->join();
        mThreads.erase(it);
    }

    wipe_SampleMap(true);
}

void AudioManager::wipe_SampleMap(bool drop_data)
{
    while(!mSampleMap.empty())
    {
        SampleMap::iterator it = mSampleMap.begin();
        Sample* smp = it->second;
        mSampleMap.erase(it);
        if (drop_data) {
            smp->drop();
            delete  smp;
        }
    }
}

void AudioManager::swap_SampleMap(SampleMap& new_map)
{
    mSampleMap.swap(new_map);
    for(auto node : mSampleMap)
        node.second->stop(); // Reset loop position to beginning
}

bool AudioManager::play(ulong sample, uchar track, float vol, float pan, bool loop)
{
    TrackMap::iterator  T = mTrackMap .find(track );
    if (T == mTrackMap .end()) return false;
    SampleMap::iterator S = mSampleMap.find(sample);
    if (S == mSampleMap.end()) return false;

    Sample* s = S->second;
    Track * t = T->second;

    std::lock_guard<std::mutex> lock(mMutex);
    s->play(vol, pan, loop);

    VoiceMap::iterator  v = mVoiceMap.find(s);
    if (v == mVoiceMap.end()) // New voice
        mVoiceMap[s] = t;
    else if (v->second != t) // Change destination track
        v->second = t;

    return true;
}


bool AudioManager::update()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mOutputDevice.getFIFOBuffer().size() > mMasterTrack.getOutput().getSampleCount())
        return false;

    // Pull all tracks
    std::list<Sample*> dead;

    for(VoiceMapNode& v : mVoiceMap)
        if (v.first->is_active())
            v.second->pull(v.first);
        else
            dead.push_back(v.first);

    // Clean-up
    for(Sample* s : dead)
        mVoiceMap.erase(mVoiceMap.find(s));

    // Process stuff
    mMasterTrack.pull();
    mMasterTrack.flip();

    // Push to output device buffer
    mOutputDevice.getFIFOBuffer_mutex().lock();
    mMasterTrack.push(mOutputDevice.getFIFOBuffer());
    mOutputDevice.getFIFOBuffer_mutex().unlock();

    return true;
}

void AudioManager::attach_thread(std::thread* thread_ptr)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mThreads.push_back(thread_ptr);
}
