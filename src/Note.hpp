//  Note.h :: Standard Note objects
//  Copyright 2013 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef NOTE_H
#define NOTE_H

#include <stdexcept>
#include <utility>
#include "Note.hh"

class Note_Single : public Note
{
protected:
    unsigned    mSampleID;
    float       mVol;
    float       mPan;

public:
    Note_Single(ENKey key, TTime time, unsigned sampleID, float vol = 1.0f, float pan = 0.0f) :
        Note(key, time),
        mSampleID(sampleID),
        mVol(vol),
        mPan(pan)
    { }

    inline float const & getVol() const { return mVol; }
    inline float const & getPan() const { return mPan; }

    inline unsigned const & getSampleID() const { return mSampleID; }

    void init  (UI::Tracker const &);
    void render(UI::Tracker const &, clan::Canvas &) const;
    void update(UI::Tracker const &, KeyStatus const &);
};

class Note_Long : public Note
{
protected:
    TTime       mBTime , mETime;

    unsigned    mBSID, mESID;
    float       mVol, mPan;

    long        mBTick , mETick;
    JScore      mBScore, mEScore;

private:
    bool        mHasEndPoint;

    void calc_score();

public:
    Note_Long(
        ENKey key,
        TTime bTime, TTime eTime,
        unsigned bSID, unsigned eSID,
        float vol = 1.0f, float pan = 0.0f
    ) : Note(key, bTime),
        mBTime (bTime)   , mETime (eTime),
        mBSID  (bSID)    , mESID  (eSID),
        mVol   (vol)     , mPan   (pan),
        mBTick (0)       , mETick (0),
        mBScore(JScore()), mEScore(JScore()),
        mHasEndPoint(true)
    { }

    Note_Long(
        ENKey key ,
        TTime time,
        unsigned sampleID,
        float vol = 1.0f, float pan = 0.0f
    ) : Note(key, time),
        mBTime (time)    , mETime (time),
        mBSID  (sampleID), mESID  (sampleID),
        mVol   (vol)     , mPan   (pan),
        mBTick (0)       , mETick (0),
        mBScore(JScore()), mEScore(JScore()),
        mHasEndPoint(false)
    { }

    Note_Long(
        Note_Single const &begin,
        Note_Single const &end,
        float vol = 1.0f, float pan = 0.0f
    ) : Note(begin.getKey(), begin.getTime()),
        mBTime (begin.getTime())    , mETime (end.getTime()),
        mBSID  (begin.getSampleID()), mESID  (end.getSampleID()),
        mVol   (vol)     , mPan   (pan),
        mBTick (0)       , mETick (0),
        mBScore(JScore()), mEScore(JScore()),
        mHasEndPoint(true)
    { }

    inline std::pair<TTime,TTime> getTime() const
    {
        return std::pair<TTime, TTime>(mBTime, mETime);
    }
    inline bool attach_release(TTime const &time, unsigned const &sampleID)
    {
        if (mHasEndPoint)
            return false;

        mETime = time;
        mESID  = sampleID;
        mHasEndPoint = true;
        return true;
    }

    void init  (UI::Tracker const &);
    void render(UI::Tracker const &, clan::Canvas&) const;
    void update(UI::Tracker const &, const KeyStatus&);
};


typedef std::list< Note_Single* >   SingleNoteList;
typedef std::list< Note_Long  * >   LongNoteList;

/** Merge two lists of single notes into a long note list.
*/
NoteList zip_to_long_notes(SingleNoteList const &HNL, SingleNoteList const &RNL);
#endif
