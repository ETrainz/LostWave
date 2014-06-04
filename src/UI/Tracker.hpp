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
    using RankScoreMap  = std::map< EJRank, int >;
    using KeyCode       = InputManager::KeyCode;

    /** Operational context for note lanes / keys.
     *
     * #TODO Allow multiple input codes.
     * #TODO Put KeyStatus here and ditch InputManager.
     * #TODO Allow indiviudal key theming. (maybe put this into a separate model?)
     */
    struct Channel
    {
        ENKey       key;    //! Note channel key
        KeyCode     code;   //! Player input key code

        Note      * note;   //! Currently focused note.

        ////    Graphical state variables    ///////////////////////////
        clan::Sprite    sprHit;         //! Note hit effect sprite
        clan::Colorf    clrLaneKeyOn;   //! Lane color when key is pressed
    };

    //  Channel object container type
    using ChannelList   = std::list < Channel >;

private:
    ////    Judgement and Scoring    ///////////////////////////////////
    Judge           mJudge;

    RankScoreMap    mRankScores;
    uint            mCombo, mMaxCombo;


    ////    Chart    ///////////////////////////////////////////////////
    Chart*          mChart;


    ////    Clocks and Timing    ///////////////////////////////////////
    TClock      *   mClock;
    TTime const &   mTime;
    long            mCurrentTick;

    bool            mChartEnded;
    uint            mMeasureIterStart;


    ////    Note Lane Channeling    ////////////////////////////////////
    ChannelList     mChannelList;

    ////    Input Manager    ///////////////////////////////////////////
    InputManager    mIM;


    ////    Graphics    ///////////////////////////////////////////////
    clan::Texture2D mT_Hit;
    NoteList        mRenderList;


    ////    Modifiers    ///////////////////////////////////////////////
    bool            mAutoPlay;
    float           mSpeedX;



public:
    Tracker
        ( Game        * game
        , recti const & area
        , Judge const & judge
        , Chart       * chart
        , ChannelList const & channels
        , std::string const & ref_label = ""
        , TClock            * ref_clock = nullptr
        );

    inline Judge   const &cgetJudge() const { return mJudge; }
    inline Judge         & getJudge()       { return mJudge; }
    inline TClock  const *cgetClock() const { return mClock; }
    inline TClock        * getClock()       { return mClock; }
    inline long    const & getCurrentTick() const { return mCurrentTick; }

    inline float   const & getSpeedX() const { return mSpeedX; }

    inline void setSpeedX(float value = 1.0f) { mSpeedX = value; } // #TODO validation checks
    inline void setAutoplay(bool value = true) { mAutoPlay = value; }

    point2i translate(ENKey const &key, long const &time) const;

    long compare_ticks(TTime const &a, TTime const &z) const;

    // TODO Add ability to start from a different time point

    void start();
    void update();

    void loop_Params(ParamEventList &params);
    void loop_Notes (NoteList &notes, uint &cache);

    void process_input();

    ////    Depended by Note    ///////////////////////////////////////
    rectf getDrawRect(ENKey const &key, long const &time) const;

    ////    GUI Component Callbacks    ////////////////////////////////
    void render(clan::Canvas &canvas, recti const &clip_rect);
};

}
#endif
