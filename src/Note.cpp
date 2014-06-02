//  Note.h :: Standard Note object definitions
//  Copyright 2013 Keigen Shu

#include <future>
#include "Note.hpp"
#include "UI/Tracker.hpp"
#include "AudioManager.hpp"

AudioManager* Note::am = nullptr;

void Note_Single::init(UI::Tracker const &tracker)
{
    mScore = JScore();
    mTick  = tracker.compare_ticks(TTime(), this->getTime());
}

void Note_Single::render(UI::Tracker const &tracker, clan::Canvas &canvas) const
{
    rectf p = tracker.getDrawRect(this->getKey(), this->getTick());

    if (p.left > canvas.get_width () || p.right  < 0)
        return;
    if (p.top  > canvas.get_height() || p.bottom < 0)
        return;

    p.top    = canvas.get_height() - p.top;
    p.bottom = canvas.get_height() - p.bottom;

    clan::Colorf color;

    switch (getKey())
    {
        case ENKey::NOTE_P1_1:
        case ENKey::NOTE_P1_3:
        case ENKey::NOTE_P1_5:
        case ENKey::NOTE_P1_7:
            color = clan::Colorf::white;
            break;
        case ENKey::NOTE_P1_2:
        case ENKey::NOTE_P1_6:
            color = clan::Colorf::cyan;
            break;
        case ENKey::NOTE_P1_4:
            color = clan::Colorf::gold;
            break;
        default:
            color = clan::Colorf::white;
    }

    canvas.fill_rect(p, color);
}

void Note_Single::update(UI::Tracker const &tracker, const KeyStatus &stat)
{
    JScore score = tracker.cgetJudge().judge(this->getTick() - tracker.getCurrentTick());

    switch(stat)
    {
        case KeyStatus::AUTO:
            am->play(mSampleID, ENKey_toInteger(getKey()), mVol, mPan);
            mScore = JScore( AUTO, 0, score.delta );
            mDead  = true;
            return;
        case KeyStatus::ON :
            am->play(mSampleID, ENKey_isPlayer1(getKey()) ? 1 : 2, mVol, mPan);
            if (score.rank == EJRank::NONE)
            {
                return;
            } else {
                mScore = score;
                mDead  = true;
                return;
            }

        case KeyStatus::OFF:
        case KeyStatus::LOCKED:
        default:
            if (score.rank == EJRank::MISS) // Too late to hit.
            {
                mScore = score;
                mDead  = true;
                return;
            } else {
                return;
            }
    }
}


////    Note_Long    //////////////////////////////////////////////////

void Note_Long::init(UI::Tracker const &tracker)
{
    mScore = JScore();
    mBTick = tracker.compare_ticks(TTime(), mBTime);
    mETick = tracker.compare_ticks(TTime(), mETime);
}

void Note_Long::render(UI::Tracker const &tracker, clan::Canvas &canvas) const
{
    recti pb = tracker.getDrawRect(this->getKey(), mBTick);
    recti pe = tracker.getDrawRect(this->getKey(), mETick);

    // Skip unused
    if (pb.left > canvas.get_width () || pb.right  < 0 ||
        pe.left > canvas.get_width () || pe.right  < 0)
        return;
    if (pb.top  > canvas.get_height() || pe.bottom < 0)
        return;

    // Flip around
    pb.top    = canvas.get_height() - pb.top;
    pb.bottom = canvas.get_height() - pb.bottom;
    pe.top    = canvas.get_height() - pe.top;
    pe.bottom = canvas.get_height() - pe.bottom;

    clan::Colorf color;

    switch (getKey())
    {
        case ENKey::NOTE_P1_1:
        case ENKey::NOTE_P1_3:
        case ENKey::NOTE_P1_5:
        case ENKey::NOTE_P1_7:
            color = clan::Colorf::white;
            break;
        case ENKey::NOTE_P1_2:
        case ENKey::NOTE_P1_6:
            color = clan::Colorf::cyan;
            break;
        case ENKey::NOTE_P1_4:
            color = clan::Colorf::gold;
            break;
        default:
            color = clan::Colorf::white;
    }

    clan::Colorf body, head;

    if (mBScore.rank == EJRank::AUTO) {
        body = head = clan::Colorf::gold;
        body.a = 0.8f;
    } else if (mBScore.rank == EJRank::MISS || mEScore.rank == EJRank::MISS) {
        body = head = clan::Colorf::red;
        body.a = 0.4f;
        head.a = 0.8f;
    } else if (mBScore.rank == EJRank::NONE) {
        body = head = color;
        body.a = 0.8f;
    } else if (mEScore.rank == EJRank::NONE) {
        body = head = clan::Colorf::green;
        body.a = 0.8f;
    } else {
        // Release a little too early
        body = head = clan::Colorf::lightblue;
        body.a = 0.2f;
        head.a = 0.4f;
    }

    canvas.fill_rect(pe.left, pe.top, pb.right, pb.bottom, body);
    canvas.fill_rect(pb, head);
    canvas.fill_rect(pe, head);
}


void Note_Long::calc_score()
{
    mScore.rank  = mEScore.rank;
    mScore.score = mEScore.score + mBScore.score;
    mScore.delta = mEScore.delta + mBScore.delta;
}


void Note_Long::update(UI::Tracker const &tracker, KeyStatus const &stat)
{
    JScore b_temp = JScore();
    JScore e_temp = JScore();

    b_temp = tracker.cgetJudge().judge(mBTick - tracker.getCurrentTick());
    e_temp = tracker.cgetJudge().judge(mETick - tracker.getCurrentTick());


    // Remove from key-lock context if score is already set.
    // But stay alive if not past deletion point.
    if (mScore.rank != EJRank::NONE) {
        mDead = (e_temp.rank == EJRank::MISS) ? true : mDead;
        return;
    }

    //// WAIT -> Starting point not hit yet. Respond to key status.
    //// LIVE -> Starting point hit, but end point hasn't. Respond to key status.
    //// DONE -> Note::score is set, but not dead as we still need to render graphics.
    //// DEAD -> Note::dead  is set.
    //// TODO Make this prettier and less redundant.
    switch(stat)
    {
        case KeyStatus::AUTO: // [DONE] Autoplay note.
            if (mBScore.rank == EJRank::NONE) {
                am->play(mBSID, ENKey_toInteger(getKey()), mVol, mPan);
                mBScore = JScore( EJRank::AUTO, 0, b_temp.delta );
            }

            if (e_temp.rank == EJRank::MISS) {
                mEScore = JScore( EJRank::AUTO, 0, e_temp.delta );
                mScore  = JScore( EJRank::AUTO, 0, 0 );
                mDead = true;
            }

            return;

        case KeyStatus::LOCKED: // Holding Key
            if (mEScore.rank == EJRank::NONE) {         // Unscored end
                if (mBScore.rank != EJRank::NONE
                &&  mBScore.rank != EJRank::MISS
                &&  mBScore.rank != EJRank::AUTO) {     // Scored starting point
                    assert(mScore.rank == NONE && "Note logic leak.");
                    if (e_temp.rank == MISS) {          // [DONE] Too late to release
                        mEScore = e_temp;
                        calc_score();
                        mDead = true;
                        return;
                    } else {                            // [LIVE] Still waiting for end point
                        return;
                    }
                } else if (mBScore.rank == EJRank::MISS
                        || mBScore.rank == EJRank::AUTO) {      // Missed starting point
                    mEScore = mBScore;                  // This should not be needed, but someone kept forgetting to set mEScore somewhere.
                    calc_score();
                    return;
                } else {                                        // Unscored starting point
                    if (b_temp.rank == EJRank::MISS) {  // [DONE] Missed starting point.
                        mBScore = b_temp;
                        mEScore = b_temp;
                        calc_score();
                        return;
                    } else {                            // [WAIT] Still have the time to respond.
                        return;
                    }
                }
                // Key lock belongs to another note... Same effect as being OFF.
            } else { return; }                                  // [DONE] Scored end
            break;

        case KeyStatus::OFF : // Have not hit anything OR released key.
            if (mBScore.rank == EJRank::NONE) {     // Unscored starting point; not active yet.
                if (b_temp.rank == EJRank::MISS) {  // [DONE] Missed starting point.
                    mBScore = b_temp;
                    mEScore = b_temp;
                    calc_score();
                    return;
                } else {                            // [WAIT] Still have the time to respond.
                    return;
                }
            } else if (mBScore.rank == EJRank::MISS
                    || mBScore.rank == EJRank::AUTO) {      // Starting point was scored MISS or AUTO
                assert(mBScore.rank == mEScore.rank);       // Starting point and ending points must be equal.
                if (e_temp.rank == EJRank::MISS) {  // [DEAD] Past target time
                    mDead = true;
                    return;
                } else {                            // [DONE] Not past target time
                    return;
                }
            } else {                                // Starting point was scored.
                if (mEScore.rank == EJRank::NONE) {         // Ending point hasn't, so the player was holding this.
                    if (e_temp.rank == EJRank::NONE) {              // [DONE] Release too early
                        mEScore = JScore( EJRank::MISS, 0, e_temp.delta );
                        calc_score();
                        return;
                    } else {                                        // [DEAD] Release at the right time
                        mEScore = e_temp;
                        calc_score();
                        return;
                    }
                } else {                                    // Ending point was scored.
                    if (e_temp.rank == EJRank::MISS) {              // [DEAD] Past target time
                        mDead = true;
                        return;
                    } else {                                        // [DONE] Not past target time
                        return;
                    }
                }
            }

        case KeyStatus::ON: // Just hit the key or rehit after miss.
            if (mBScore.rank == EJRank::NONE) {             // Starting point was not hit
                am->play(mBSID, ENKey_isPlayer1(getKey()) ? 1 : 2, mVol, mPan);  // Play sound.
                if (b_temp.rank == EJRank::NONE) {                  // [WAIT] Hit too early
                    return;
                } else if (b_temp.rank == EJRank::MISS
                        || b_temp.rank == EJRank::AUTO) {           // [DONE] Rare case of MISS right when the key is hit.
                    mBScore = b_temp;
                    mEScore = b_temp;
                    calc_score();
                    return;
                } else {                                            // [LIVE] Staring point scores!
                    mBScore = b_temp;
                    return;                                     // DO NOT CLEAR FROM ACTIVE QUEUE
                }
            } else if (mEScore.rank == EJRank::NONE) {      // Starting point was hit, ending point hasn't
                if (e_temp.rank == EJRank::MISS) {                  // [DEAD] Past target time
                    mEScore = e_temp;
                    calc_score();
                    mDead = true;
                    return;
                } else {                                            // [DONE] Not past target time
                    am->play(mBSID, ENKey_isPlayer1(getKey()) ? 1 : 2, mVol, mPan);             // Play sound.
                    return;
                }
            } else {
                return;
            }

        default:
            return;
    }
}

/*! Connects two lists of notes to a list of long notes.
 *
 * This function assumes that all notes being sent into this function
 * have the same key.
 *
 * This function will attempt to fix erroneous long note sequences.
 * The fixes are based on the following conditions:
 * - A HOLD note MUST be located before a RELEASE note.
 * - Both ends of the long notes MUST have identical Sample IDs, except
 *   in the case of BMS's LN_TYPE 1 mode.
 * - Switching between HOLD notes and NORMAL notes are allowed because
 *   they both make sounds.
 * - Deletion of RELEASE notes are allowed because they do not make any sound.
 */
NoteList zip_key(SingleNoteList H, SingleNoteList R)
{
    NoteList L;

    while(!H.empty())
    {
        if (R.empty())
        {
            clan::Console::write_line("Note [debug] Converting lone HOLD to NORMAL.");
            L.push_back(*(H.begin()));
            H.erase(H.begin());
        } else {
            SingleNoteList::iterator
                h = H.begin(), r = R.begin();
            Note_Single
                *hn = *h, *rn = *r;

            ////    CHECK AND FIX NOTES    ////////////////////////////
            // TODO Put note checking logic into a class.
            bool errTime = hn->getTime()     >  rn->getTime();
            bool errSmpl = hn->getSampleID() != rn->getSampleID();

            if (hn->getTime() == rn->getTime()) {
                clan::Console::write_line("Note [debug] Bad long note: Instant RELEASE.");
                clan::Console::write_line("     [---->] Deleting RELEASE.");

                if (errSmpl)
                    clan::Console::write_line("     [---->] Note: It has mis-matched samples.");

                R.erase(r); delete rn;
                continue;
            }

            if (errTime)
            {
                clan::Console::write_line("Note [debug] Bad long note: Invalid time.");
                clan::Console::write_line("     [---->] Deleting RELEASE.");

                if (errSmpl)
                    clan::Console::write_line("     [---->] Note: It has mis-matched samples.");

                // TODO If the note before `r` is a NORMAL note with the same sample ID,
                //      one may change the type of that NORMAL note to HOLD. But we don't
                //      have access to any NORMAL notes.
                R.erase(r); delete rn;
                continue;
            }

            if (errSmpl)
            {
                clan::Console::write_line("Note [debug] Bad long note: Sound mismatch.");

                SingleNoteList::iterator t = h;
                if ((++t) != H.end())
                {
                    if ((*t)->getTime() < rn->getTime())
                    {
                        clan::Console::write_line("     [---->] Converting HOLD to NORMAL since the next hold note can fit.");
                        L.push_back(hn);
                        H.erase(h);
                        continue;
                    }
                }

                clan::Console::write_line("     [---->] Deleting RELEASE.");
                R.erase(r); delete rn;
                continue;
            }

            L.push_back(new Note_Long(*hn, *rn, hn->getVol(), hn->getPan()));
            H.erase(h); delete hn;
            R.erase(r); delete rn;
        }
    }

    return L;
}

NoteList zip_to_long_notes(SingleNoteList const &HNL, SingleNoteList const &RNL)
{
    auto Hmap = split_notes_by_key(HNL);
    auto Rmap = split_notes_by_key(RNL);

    std::list<std::future<NoteList>> futures;

    for(auto node : Hmap)
        futures.push_back(
                std::async(
                    std::launch::async,
                    zip_key, Hmap[node.first], Rmap[node.first]
                    )
                );

    NoteList L;

    while(!futures.empty())
    {
        auto it = futures.begin();
        NoteList l = it->get();
        L.insert(L.end(), l.begin(), l.end());
        futures.erase(it);
    }

    return L;
}


