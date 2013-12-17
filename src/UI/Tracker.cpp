//  UI/Tracker.hpp :: Note tracker UI component
//  Copyright 2013 Keigen Shu

#include "Tracker.hpp"
#include "../Chrono.hpp"
#include "../Chart.hpp"
#include "../Game.hpp" // Access to game config options

namespace UI {

Tracker::Tracker(
    Game        *game ,
    recti const &area ,
    Judge const &judge,
    Chart *chart,
    std::initializer_list<KeyInputPair> show_keys,
    std::initializer_list<ENKey>        auto_keys,
    std::string  const &label,
    TClock             *clock
) : clan::GUIComponent(
        reinterpret_cast<clan::GUIComponent*>(game),
        { area, false },
        "tracker"),
    mJudge(judge),
    mRankScores({
            {EJRank::PERFECT, 0},
            {EJRank::COOL, 0},
            {EJRank::GOOD, 0},
            {EJRank::BAD , 0},
            {EJRank::MISS, 0}
            }),
    mCombo(0), mMaxCombo(0),

    mShowKeys(),
    mAutoKeys(auto_keys),
    mChart(chart),

    mClock(clock == nullptr ? new TClock(chart->getTempo()) : clock ),
    mCurrentTick(0),

    mChartEnded(false),
    mMeasureIterStart(0),

    mKeyInputs(),
    mKeyNotes (),

    mRenderList(),

    mIM(this->get_ic()),
    mSpeedX(game->conf.get_if_else_set(
            &JSONReader::getInteger, "player.P1.speedx", 1.0,
            [] (long const &value) -> bool { return value > 0.25; }
            )),

    mTime(mClock->cgetTTime())
{
    for(auto key : show_keys) {
        mShowKeys.push_back(key.first);
        mKeyInputs .insert(key);
        mIM.try_lock(key.second);
    }

    // TODO Remove duplicate keys.
    // TODO Remove auto_keys that are not in show_keys.

    mJudge.calculateTiming(mClock->getTempo_mspt());

    set_constant_repaint(true);

    func_render().set(this, &Tracker::render);
    func_input ().set(this, &Tracker::process_input);

    // Pre-calculate tick values for notes.
    for(Measure* measure : mChart->getSequence())
        for(Note* note : measure->getNotes())
            note->init(*this);
}


long Tracker::compare_ticks(TTime const &a, TTime const &z) const
{
    return mChart->compare_ticks(a,z);
}

// FIXME This thing doesn't work right on low BPM environments.
//       Test it on The Festival of Ghost 2 and you'll see it fail.
point2i Tracker::translate(ENKey const &ref, long const &time) const
{
    int x = -1;
    int y = time - mCurrentTick;

    KeyList::const_iterator it = mShowKeys.cbegin();
    for(int i=0; it != mShowKeys.cend(); it++, i++)
    {
        if (ref == *it)
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


void Tracker::render(clan::Canvas& canvas, const recti& clip_rect)
{
    update();

    float z = canvas.get_height();
    float p = z - mSpeedX * (mJudge.cgetTP());
    float c = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC());
    float g = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC() + mJudge.cgetTG());
    float b = z - mSpeedX * (mJudge.cgetTP() + mJudge.cgetTC() + mJudge.cgetTG() + mJudge.cgetTB());

    canvas.fill_rect({0, 0, 168, z}, { 1.0f, 1.0f, 1.0f, 0.1f});
    canvas.fill_rect({0, p, 168, z}, { 0.0f, 0.5f, 1.0f, 0.1f});
    canvas.fill_rect({0, c, 168, p}, { 0.0f, 1.0f, 0.5f, 0.1f});
    canvas.fill_rect({0, g, 168, c}, { 0.5f, 1.0f, 0.0f, 0.1f});
    canvas.fill_rect({0, b, 168, g}, { 1.0f, 0.5f, 0.0f, 0.1f});

    for(auto pair : mKeyInputs)
        if (mIM.isOn(pair.second))
        {
            rectf rect  = getDrawRect(pair.first, 0);
            rect.top    = 0;
            rect.bottom = canvas.get_height();
            canvas.fill_rect(rect, { 1.0f, 1.0f, 1.0f, 0.1f });
        }

    for(Note* note : mRenderList)
        note->render(*this, canvas);

    mRenderList.clear();
}


void Tracker::start() { mClock->start(); }

void Tracker::update()
{
    mClock->update();
    mCurrentTick = mChart->compare_ticks(_0TTime, mTime);

    if (mTime.measure > mChart->getMeasures()) {
        mChartEnded = true;
        if (true && true)
            // Wait for all sounds to stop
            // Wait for other players to finish playing
            exit_with_code(0);
            return;
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

    KeyList clearing_list;

    for(auto pair : mKeyNotes)
    {
        ENKey const key  = pair.first;
        Note* const note = pair.second;

        note->update(*this, mIM.getKey(mKeyInputs[key]));

        if (note->isScored())
        {
            clearing_list.push_back(key);

            JScore score = note->getScore();
            mRankScores[score.rank] += 1;
            if (score.rank != MISS && score.rank != BAD)
                mCombo += 1;
            else
                mCombo  = 0;
        }
    }

    mMaxCombo = std::max(mCombo, mMaxCombo);

    for(auto key : clearing_list)
        mKeyNotes.erase(key);

    for(auto pair : mKeyInputs)
        if (mIM.isOn(pair.second))
            mIM.try_lock(pair.second);
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
                if (ENKey_isAutoPlay(note->getKey()) ||
                    mAutoKeys.find(note->getKey()) != mAutoKeys.end()
                        ) {
                    if (note->getTime() <= mTime)
                        note->update(*this, KeyStatus::AUTO);
                    //// Notes are already aligned by time, so this is correct
                } else if (std::find(mShowKeys.begin(), mShowKeys.end(), note->getKey()) == mShowKeys.end()) {
                    if (note->getTime() <= mTime) {
                        note->update(*this, KeyStatus::ON);
                        note->update(*this, KeyStatus::OFF);
                    }
                } else if (mKeyNotes.find(note->getKey()) == mKeyNotes.end()) {
                    mKeyNotes.insert(std::make_pair(note->getKey(), note));
                }
            } else {
                // Make note do whatever it needs to die.
                note->update(*this, KeyStatus::OFF);
            }
        } // ELSE IGNORE THE DEAD
    }
}

bool Tracker::process_input(const clan::InputEvent& event)
{
    /****/ if (event.device.get_type() == clan::InputDevice::Type::keyboard)
    {
        // dump_event(event, "Tracker");
        /****/ if (event.type == clan::InputEvent::Type::pressed)
        {
            switch (event.id)
            {
            }

            return false;
        } else if (event.type == clan::InputEvent::Type::released) {
            switch (event.id)
            {
                case clan::InputCode::keycode_escape:
                    exit_with_code(0);
                    return true;
            }
        }
    } else if (event.device.get_type() == clan::InputDevice::Type::pointer) {

    }

    return false;
}

}