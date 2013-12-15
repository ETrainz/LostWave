//  Note.h :: Standard Note objects
//  Copyright 2013 Keigen Shu

#ifndef NOTE_H
#define NOTE_H

#include <stdexcept>
#include <utility>
#include "Note.hh"

class Note_Single : public Note
{
protected:
    unsigned sample_id;

    float vol;
    float pan;

public:
    Note_Single(TTime Time, ENKey Key, unsigned SampleID, float Vol = 1.0f, float Pan = 0.0f) :
        Note(Key, Time),
        sample_id(SampleID),
        vol(Vol),
        pan(Pan)
        { }
    virtual ~Note_Single() {}

    float getVol() const { return vol; }
    float getPan() const { return pan; }

    unsigned getSampleID() const { return sample_id; }
    void init  (UI::Tracker const &t);
    void render(UI::Tracker const &t, clan::Canvas &canvas) const;
    void update(UI::Tracker const &t, KeyStatus const &k);
};

class Note_Long : public Note
{
protected:
    TTime b_time;
    TTime e_time;
    long  b_tick;
    long  e_tick;

    unsigned b_sample_id;
    unsigned e_sample_id;

    float vol, pan;

    JScore b_score;
    JScore e_score;

    bool hasEndPoint;

    inline void calculateScore() {
        score.rank  = e_score.rank;
        score.score = e_score.score + b_score.score;
        score.delta = e_score.delta + b_score.delta;
    }

public:
    Note_Long(
            TTime bTime,
            TTime eTime,
            ENKey Key,
            unsigned bSampleID,
            unsigned eSampleID,
            float vol,
            float pan
            ) :
        Note(Key, bTime),
        b_time(bTime),
        e_time(eTime),
        b_tick(0),
        e_tick(0),
        b_sample_id(bSampleID),
        e_sample_id(eSampleID),
        vol(vol), pan(pan),
        b_score(JScore_NONE),
        e_score(JScore_NONE),
        hasEndPoint(true)
        { }

    Note_Long(
            TTime Time,
            ENKey Key,
            unsigned SampleID,
            float vol,
            float pan
            ) :
        Note(Key, Time),
        b_time(Time),
        e_time(Time),
        b_tick(0),
        e_tick(0),
        b_sample_id(SampleID),
        e_sample_id(SampleID),
        vol(vol), pan(pan),
        b_score(JScore_NONE),
        e_score(JScore_NONE),
        hasEndPoint(false)
        { }

    Note_Long(
            Note_Single const &begin,
            Note_Single const &end,
            float _vol = 1.0f,
            float _pan = 0.0f
            ) :
        Note(begin.getKey(), begin.getTime()),
        b_time(begin.getTime()),
        e_time(end.getTime()),
        b_tick(0),
        e_tick(0),
        b_sample_id(begin.getSampleID()),
        e_sample_id(end.getSampleID()),
        vol(_vol), pan(_pan),
        b_score(JScore_NONE),
        e_score(JScore_NONE),
        hasEndPoint(true)
        {}

    virtual ~Note_Long() {}
    inline std::pair<TTime,TTime> getTime() const { return std::pair<TTime, TTime>(b_time, e_time); }
    inline bool attach_release(TTime Time, unsigned SampleID)
    {
        if (hasEndPoint) return false;
        e_time      = Time;
        e_sample_id = SampleID;
        hasEndPoint = true;
        return true;
    }

    void init  (UI::Tracker const &t);
    void render(UI::Tracker const &t, clan::Canvas &canvas) const;
    void update(UI::Tracker const &t, KeyStatus const &k);
};


typedef std::list< Note_Single* >   SingleNoteList;
typedef std::list< Note_Long  * >   LongNoteList;

/** Merge two lists of single notes into a long note list.
*/
NoteList zip_to_long_notes(SingleNoteList const &HNL, SingleNoteList const &RNL);
#endif
