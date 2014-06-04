#define nothing {} while (false)

#include "Tracker.hpp"
#include "../Chrono.hpp"
#include "../Chart.hpp"
#include "../Game.hpp" // Access to game config options

namespace UI {

Tracker::Tracker
    ( Game        * game
    , recti const & area
    , Judge const & judge
    , Chart       * chart
    , ChannelList const & channels
    , std::string const & ref_label
    , TClock            * ref_clock
) : clan::GUIComponent(reinterpret_cast<clan::GUIComponent *>(game), "Tracker")

    , mJudge(judge)
    , mRankScores(
        { { EJRank::PERFECT, 0 }
        , { EJRank::COOL, 0 }
        , { EJRank::GOOD, 0 }
        , { EJRank::BAD , 0 }
        , { EJRank::MISS, 0 }
        })
    , mCombo(0), mMaxCombo(0)

    , mChart(chart)

    , mClock(ref_clock == nullptr ? new TClock(mChart->getTempo()) : ref_clock) // #TODO Fix mClock memory leak.
    , mTime (mClock->getTTime())
    , mCurrentTick(0)
    , mChartEnded(false)
    , mMeasureIterStart(0)

    , mChannelList()

    , mIM(this->get_ic())

    , mT_Hit(   // #TODO Add this functionality into JSONReader
            this->get_canvas().get_gc(),
            game->skin.get_or_set(
                &JSONReader::getString,
                "theme.play-1p.note.tex_hit.path",
                std::string{ "Theme/Note_Hit.png" }
                ))
    , mT_Hit_Rank(   // #TODO Add this functionality into JSONReader
            this->get_canvas().get_gc(),
            game->skin.get_or_set(
                &JSONReader::getString,
                "theme.play-1p.note.tex_hit_rank.path",
                std::string{ "Theme/Note_Hit_Rank.png" }
                ))

    , mRenderList()

    , mAutoPlay (game->conf.get_or_set(
            &JSONReader::getBoolean, "player.P1.autoplay", false
            ))
    , mSpeedX   (game->conf.get_if_else_set(
        &JSONReader::getInteger, "player.P1.speedx", 1.0,
        [] (long const &value) -> bool { return value > 0.25; }
        ))
{
    ////    Setup Graphics

    mI_Hit_Rank[0] = clan::Image(mT_Hit_Rank, recti{  1,  1, 45,  8 });
    mI_Hit_Rank[1] = clan::Image(mT_Hit_Rank, recti{  1, 10, 26, 17 });
    mI_Hit_Rank[2] = clan::Image(mT_Hit_Rank, recti{  1, 19, 26, 26 });
    mI_Hit_Rank[3] = clan::Image(mT_Hit_Rank, recti{  1, 28, 20, 35 });
    mI_Hit_Rank[4] = clan::Image(mT_Hit_Rank, recti{  1, 37, 26, 44 });


    ////    Setup GUI Component
    func_render().set(this, &Tracker::render);

    set_constant_repaint(true);
    set_geometry(area);

    ////    Setup channels
    clan::Canvas canvas = this->get_canvas();

    //  Safely initialize channel list
    for(auto const &elem : channels)
        mChannelList.push_back(Channel { elem.key, elem.code, nullptr, { canvas }, { 1.0f, 1.0f, 1.0f, 0.1f } });

    for(auto &elem : mChannelList)
    {
        //  #HACK Bind mapping
        mIM.try_lock    (elem.code);
        mIM.try_unlock  (elem.code);

        //  Create sprites
        elem.sprHit.add_gridclipped_frames(canvas, mT_Hit, 0, 0, 256, 256, 4, 3, 0, 0, 0);
        elem.sprHit.set_alpha(2.f / 3.f);
        elem.sprHit.set_delay(20.0f);
        elem.sprHit.set_play_loop(false);
        elem.sprHit.finish();

        //  Load LaneKeyOn colors
        //  #TODO Load from theme JSON
        switch(elem.key) {
            case ENKey::NOTE_P1_1: case ENKey::NOTE_P2_1:
            case ENKey::NOTE_P1_3: case ENKey::NOTE_P2_3:
            case ENKey::NOTE_P1_5: case ENKey::NOTE_P2_5:
            case ENKey::NOTE_P1_7: case ENKey::NOTE_P2_7:
                elem.clrLaneKeyOn = clan::Colorf::white;
                break;
            case ENKey::NOTE_P1_2: case ENKey::NOTE_P2_2:
            case ENKey::NOTE_P1_6: case ENKey::NOTE_P2_6:
                elem.clrLaneKeyOn = clan::Colorf::cyan;
                break;
            case ENKey::NOTE_P1_4: case ENKey::NOTE_P2_4:
                elem.clrLaneKeyOn = clan::Colorf::gold;
                break;
            default:
                elem.clrLaneKeyOn = clan::Colorf::pink;
                break;
        };

        elem.clrLaneKeyOn.set_alpha(0.1f);
    }


    ////    Setup note chart

    //  Calculate judgement timing
    mJudge.calculateTiming(mClock->getTempo_mspt());

    //  Initialize notes :: Calculate tick values for notes.
    for(Measure* measure : mChart->getSequence())
        for(Note* note : measure->getNotes())
            note->init(*this);
}


long Tracker::compare_ticks(TTime const &a, TTime const &z) const
{
    return mChart->compare_ticks(a,z);
}

point2i Tracker::translate(ENKey const &ref, long const &time) const
{
    int x = -1;
    int y = time - mCurrentTick;

    ChannelList::const_iterator it = mChannelList.cbegin();
    for(int i=0; it != mChannelList.cend(); it++, i++)
    {
        if (ref == it->key)
        {
            x = i;
            break;
        }
    }

    return { x, y };
}

rectf Tracker::getDrawRect(ENKey const &key, long const &time) const
{
    point2i p = translate(key, time);
    p.y = static_cast<float>(p.y) * mSpeedX;

    rectf ret(
        1+24*p.x   , p.y  ,
        1+24*p.x+22, p.y+4
    );

    if (p.x == -1)
        ret.left = -1, ret.right = -1;

    return ret;
}

inline float gfx_pop_func(float x) { return sin( powf(2.0f * M_PI * x, 1.0f / 2.0f) ); }

void Tracker::render(clan::Canvas& canvas, const recti& clip_rect)
{
    update();

    float z = get_height();
    float p = z - mSpeedX * (mJudge.cgetTP());
    float c = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC());
    float g = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC() + mJudge.cgetTG());
    float b = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC() + mJudge.cgetTG() + mJudge.cgetTB());

    canvas.fill_rect({0, 0, 168, z}, { 1.0f, 1.0f, 1.0f, 0.1f });

    canvas.fill_rect({0, p, 168, z}, { 0.0f, 0.5f, 1.0f, 0.1f });
    canvas.fill_rect({0, c, 168, p}, { 0.0f, 1.0f, 0.5f, 0.1f });
    canvas.fill_rect({0, g, 168, c}, { 0.5f, 1.0f, 0.0f, 0.1f });
    canvas.fill_rect({0, b, 168, g}, { 1.0f, 0.5f, 0.0f, 0.1f });

    if (mClock->isTStopped())
        canvas.fill_rect( { 0, 0, 168, z }, { 1.0f, 1.0f, 1.0f, 0.5f });

    canvas.push_cliprect( get_geometry() );

    for(Note *note : mRenderList)
        note->render(*this, canvas);

    mRenderList.clear();
    canvas.pop_cliprect();
    {
        std::list<point2i> cl_NRL;
        for(auto pair : mNoteRankList)
        {
            int x = 0;
            switch(pair.second) {
                case EJRank::PERFECT: x = 0; break;
                case EJRank::COOL   : x = 1; break;
                case EJRank::GOOD   : x = 2; break;
                case EJRank::BAD    : x = 3; break;
                case EJRank::MISS   : x = 4; break;
            }

            point2f pos (pair.first.x, pair.first.y);

            float z = static_cast<float>(mCurrentTick - pos.y) / static_cast<float>(mClock->getTicksPerMeasure() / 2);
            if (z > 1.0f) {
                cl_NRL.push_back(pair.first);
                continue;
            }

            pos.y = (1.0f - gfx_pop_func(z) / 2.0f) * get_height();
            mI_Hit_Rank[x].set_alpha(std::min(0.5f, (5.0f / 3.0f) * (1.0f - z)));
            mI_Hit_Rank[x].draw(canvas, { pos.x - mI_Hit_Rank[x].get_width() / 2 }, pos.y);
        }

        for (auto elem : cl_NRL)
            mNoteRankList.erase(mNoteRankList.find(elem));

        cl_NRL.clear();
    }


    for(auto &elem : mChannelList)
    {
        rectf rLane = getDrawRect(elem.key, 0);
        rLane.top       = 0;
        rLane.bottom    = get_height();

        if (mIM.isOn(elem.code))
            canvas.fill_rect(rLane, elem.clrLaneKeyOn);

        rLane.top       = get_height();
        if (elem.sprHit.is_finished() == false)
        {
            point2f const pos = alignCC(rLane, sizef{ elem.sprHit.get_frame_size(elem.sprHit.get_current_frame()) }).get_top_left();
            elem.sprHit.draw(canvas, pos.x, pos.y);
            elem.sprHit.update(15.0f);
        }
    }
}


void Tracker::start() { mClock->start(); }

void Tracker::update()
{
    mClock->update();
    mCurrentTick = mChart->compare_ticks(TTime(), mTime);

    if (mTime.measure > mChart->getMeasures()) {
        mChartEnded = true;
        if (true && true) {
            // Wait for all sounds to stop
            // Wait for other players to finish playing
            exit_with_code(0);
            return;
        }
    }

    if (mChartEnded == false)
    {
        mRenderList.clear();

        uint cache = mTime.measure;
        uint index = mMeasureIterStart;
        uint count = 0;
        do
        {
            Measure* measure = mChart->getMeasure(index);
            /****/ if (measure == nullptr) {
                count += 192; // Empty measure; skip
                continue;
            } else if (mTime.measure >  index) {
                do nothing;
            } else if (mTime.measure <  index) {
                count += measure->getTickCount();
            } else if (mTime.measure == index) {
                mClock->setTCSig(measure->getA(), measure->getB());
            }

            loop_Params(measure->getParams());
            loop_Notes (measure->getNotes (), cache);

            index += 1;
        } while (count <= (192 * 2));

        mMeasureIterStart = cache;
    }

    //  Update notes with player input
    for(auto &elem : mChannelList)
    {
        // No note in focus.
        if (elem.note == nullptr) continue;

        // Update note.
        elem.note->update(*this, mIM.getKey(elem.code));
        if (elem.note->isScored())
        {
            // Update scoring statistics
            JScore score = elem.note->getScore();
            mRankScores[score.rank] += 1;
            mNoteRankList[ point2i
                    ( getDrawRect(elem.note->getKey(), 0).get_center().x
                    , mCurrentTick
                    )] = score.rank;
            if (score.rank != MISS && score.rank != BAD) {
                mCombo += 1;
                elem.sprHit.restart();
            } else {
                mCombo  = 0;
            }

            // Remove from focus.
            elem.note = nullptr;
        }
    }

    // Lock pressed keys whether or not the player hit a note.
    for(auto &elem : mChannelList) {
        if (mIM.isOn(elem.code))
            mIM.try_lock(elem.code);
    }

    // Calculate max combo
    mMaxCombo = std::max(mCombo, mMaxCombo);

    // Process auxiliary input commands
    process_input();
}

void Tracker::loop_Params(ParamEventList &params)
{
    for(ParamEvent* param : params)
    {
        if (param->test())
            continue;

        if (param->time == mTime) {
            switch(param->param)
            {
                case EParam::EP_C_TEMPO :
                    // Update tempo
                    mClock->setTempo(param->value.asFloat);
                    mJudge.calculateTiming(mClock->getTempo_mspt());
                    break;
                case EParam::EP_C_STOP_T :
                    // Set tick-time pause
                    mClock->setTStop(param->value.asInt);
                    break;
                default:
                    printf("[warn] param event type not handled.\n");
                    break;
            }
            param->set();
        } else if (mClock->cgetITime() > param->time
                || mClock->cgetTTime() > mClock->cgetITime()
                ) {
            // Reset clock interrupt
            mClock->setITime(param->time);
        } else if (param->time < mTime) {
            printf("[warn] param event not handled on time.\n");
        }
    }
}

void Tracker::loop_Notes(NoteList &notes, uint &cache)
{
    for(Note* note : notes)
    {
        if (note->isDead() == false)
        {
            // Keep measure active
            if (cache > note->getTime().measure)
                cache = note->getTime().measure;

            // Render it.
            mRenderList.push_back(note);

            if (note->isScored() == false)
            {
                ChannelList::iterator chIter = std::find_if(
                        mChannelList.begin(), mChannelList.end(),
                        [&] (Channel const &ch) -> bool {
                            return ch.key == note->getKey();
                        });

                if (
                // It's in autoplay channel or background channel.
                    ENKey_isAutoPlay(note->getKey())
                // It's not on the list of selected keys.
                    ||  chIter == mChannelList.end()
                // AutoPlay is turned on
                    ||  mAutoPlay
                ) {
                    if (note->getTime() <= mTime) {
                        note->update(*this, KeyStatus::AUTO);

                        // Show note hit effect
                        if (chIter != mChannelList.end() && note->isDead()) {
                            mNoteRankList[ point2i
                                    ( getDrawRect(note->getKey(), 0).get_center().x
                                    , mCurrentTick
                                    )] = EJRank::AUTO;
                            chIter->sprHit.restart();
                        }
                    }
                } else if ( chIter->note == nullptr ) {
                    chIter->note = note;
                }
            } else {
                // Make note do whatever it needs to die.
                note->update(*this, KeyStatus::OFF);
            }
        } // ELSE IGNORE THE DEAD
    }
}

void Tracker::process_input() {
    if (mIM.try_lock(clan::InputCode::keycode_f11)) {
        mAutoPlay = !mAutoPlay;
    }
}

}
