//  UI/Tracker.hpp :: Note tracker UI component
//  Copyright 2013 Keigen Shu

#ifndef NOTE_TRACKER_H
#define NOTE_TRACKER_H

#include "../__zzCore.hpp"
#include "../InputManager.hpp"
#include "../Judge.hpp"

#include "../Note.hh"
#include "../ParamEvent.hpp"

class TClock;
class Chart;
class Game;

namespace UI {

/** Game <-> Note interaction interface */

class Tracker : public clan::GUIComponent
{
public:
    typedef typename std::map< ENKey, Note* >                   KeyNoteMap;
    typedef typename std::map< ENKey, InputManager::KeyCode >   KeyInputMap;
    typedef typename KeyInputMap::value_type                    KeyInputPair;
    typedef typename std::list< KeyInputPair >                  KeyInputList;
    typedef typename std::map< EJRank, int >                    RankScoreMap;

private:
    Judge           mJudge;
    RankScoreMap    mRankScores;
    uint            mCombo, mMaxCombo;

    KeyList         mShowKeys;  // Keys to show in tracker. Keys not shown are autoplayed by default.
    KeySet          mAutoKeys;  // Keys in mActiveKeys to autoplay.

    Chart*          mChart;

    TClock*         mClock;
    long            mCurrentTick;

    bool            mChartEnded;
    uint            mMeasureIterStart;

    KeyInputMap     mKeyInputs;
    KeyNoteMap      mKeyNotes;

    NoteList        mRenderList;

    InputManager    mIM;
    float           mSpeedX;

    ////    Convenience variables    //////////////////////////////////
    TTime const &   mTime;

public:
    Tracker(
        Game        *game ,
        recti const &area ,
        Judge const &judge,
        Chart       *chart,
        KeyInputList const &show_keys,
        KeyList      const &auto_keys,
        std::string  const &label = "",
        TClock             *clock = nullptr
    );

    inline Judge   const &cgetJudge() const { return mJudge; }
    inline Judge         & getJudge()       { return mJudge; }
    inline TClock  const *cgetClock() const { return mClock; }
    inline TClock        * getClock()       { return mClock; }
    inline long    const & getCurrentTick() const { return mCurrentTick; }
    inline KeyList const & getActiveKeys () const { return mShowKeys; }

    inline float   const & getSpeedX() const { return mSpeedX; }

    point2i translate(ENKey const &key, long const &time) const;

    long compare_ticks(TTime const &a, TTime const &z) const;

    // TODO Add ability to start from a different time point

    void start();
    void update();

    void loop_Params(ParamEventList &params);
    void loop_Notes (NoteList &notes, uint &cache);

    ////    Depended by Note    ///////////////////////////////////////
    rectf getDrawRect(ENKey const &key, long const &time) const;

    ////    GUI Component Callbacks    ////////////////////////////////
    void render(clan::Canvas &canvas, recti const &clip_rect);
    bool process_input(clan::InputEvent const &event);

};

}
#endif
