//  Note.h :: Standard Note object definitions
//  Copyright 2013 Keigen Shu

#include <future>
#include "Note.hpp"
#include "UI/Tracker.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"

AudioManager* Note::am = nullptr;

void Note_Single::init(UI::Tracker const &t)
{
    score = JScore_NONE;
    tick  = t.compare_ticks(_0TTime, time);
}

void Note_Single::render(UI::Tracker const &t, clan::Canvas &canvas) const
{
    rectf p = t.getDrawRect(key, tick);
    if (p.left > canvas.get_width () || p.right  < 0)
        return;
    if (p.top  > canvas.get_height() || p.bottom < 0)
        return;

    p.top    = canvas.get_height() - p.top;
    p.bottom = canvas.get_height() - p.bottom;

    canvas.fill_rect(p, clan::Colorf::white);
}

void Note_Single::update(UI::Tracker const &t, KeyStatus const &k)
{
    JScore temp = t.cgetJudge().judge(tick - t.getCurrentTick());

    switch (k)
    {
        case KeyStatus::AUTO:
            am->play(sample_id, 0, vol, pan);
            score = JScore ( AUTO, 0, temp.delta );
            dead  = true;
            return;
        case KeyStatus::ON :
            am->play(sample_id, 1, vol, pan);
            if (temp.rank == EJRank::NONE) {
                return;
            } else {
                score = temp;
                dead  = true;
                return;
            }

        case KeyStatus::OFF:
        case KeyStatus::LOCKED:
        default:
            if (temp.rank == EJRank::MISS) { // Too late to hit.
                score = temp;
                dead  = true;
                return;
            } else {
                return;
            }
    }
}


void Note_Long::init(UI::Tracker const &t)
{
    score = JScore_NONE;
    b_tick = t.compare_ticks(_0TTime, b_time);
    e_tick = t.compare_ticks(_0TTime, e_time);
}

void Note_Long::render(UI::Tracker const &t, clan::Canvas &canvas) const
{
    recti pb = t.getDrawRect(key, b_tick);
    recti pe = t.getDrawRect(key, e_tick);

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

    clan::Colorf body, head;

    if (b_score.rank == EJRank::AUTO) {
        body = head = clan::Colorf::gold;
        body.a = 0.8f;
    } else if (b_score.rank == EJRank::MISS || e_score.rank == EJRank::MISS) {
        body = head = clan::Colorf::red;
        body.a = 0.4f;
        head.a = 0.8f;
    } else if (b_score.rank == EJRank::NONE) {
        body = head = clan::Colorf::white;
        body.a = 0.8f;
    } else if (e_score.rank == EJRank::NONE) {
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

void Note_Long::update(UI::Tracker const &t, KeyStatus const &k)
{
    JScore b_temp = JScore_NONE, e_temp = JScore_NONE;

    b_temp = t.cgetJudge().judge(b_tick - t.getCurrentTick());
    e_temp = t.cgetJudge().judge(e_tick - t.getCurrentTick());


    // Remove from key-lock context if score is already set.
    // But stay alive if not past deletion point.
    if (score.rank != EJRank::NONE) {
        dead = (e_temp.rank == EJRank::MISS) ? true : dead;
        return;
    }

    //// WAIT -> Starting point not hit yet. Respond to key status.
    //// LIVE -> Starting point hit, but end point hasn't. Respond to key status.
    //// DONE -> Note::score is set, but not dead to render graphics.
    //// DEAD -> Note::dead  is set.
    //// TODO Make this prettier and less redundant.
    switch (k)
    {
        case KeyStatus::AUTO: // [DONE] Autoplay note.
            if (b_score.rank == EJRank::NONE) {
                am->play(b_sample_id, 0, vol, pan);
                b_score = JScore ( EJRank::AUTO, 0, b_temp.delta );
            }

            if (e_temp.rank == EJRank::MISS) {
                e_score = JScore ( EJRank::AUTO, 0, e_temp.delta );
                score = JScore ( EJRank::AUTO, 0, 0 );
                dead = true;
            }

            return;

        case KeyStatus::LOCKED: // Holding Key
            if (e_score.rank == EJRank::NONE) {         // Unscored end
                if (b_score.rank != EJRank::NONE
                        &&  b_score.rank != EJRank::MISS
                        &&  b_score.rank != EJRank::AUTO) {     // Scored starting point
                    assert(score.rank == NONE && "Note logic leak.");
                    if (e_temp.rank == MISS) {          // [DONE] Too late to release
                        e_score = e_temp;
                        calculateScore();
                        dead = true;
                        return;
                    } else {                            // [LIVE] Still waiting for end point
                        return;
                    }
                } else if (b_score.rank == EJRank::MISS
                        || b_score.rank == EJRank::AUTO) {      // Missed starting point
                    e_score = b_score;                  // This should not be needed, but someone kept forgetting to set e_score somewhere.
                    calculateScore();                   // [????]
                    return;
                } else {                                        // Unscored starting point
                    if (b_temp.rank == EJRank::MISS) {  // [DONE] Missed starting point.
                        b_score = b_temp;
                        e_score = b_temp;
                        calculateScore();
                        return;
                    } else {                            // [WAIT] Still have the time to respond.
                        return;
                    }
                }
                // Key lock belongs to another note... Same effect as being OFF.
            } else { return; }                                  // [DONE] Scored end
            break;

        case KeyStatus::OFF : // Have not hit anything OR released key.
            if (b_score.rank == EJRank::NONE) {     // Unscored starting point; not active yet.
                if (b_temp.rank == EJRank::MISS) {  // [DONE] Missed starting point.
                    b_score = b_temp;
                    e_score = b_temp;
                    calculateScore();
                    return;
                } else {                            // [WAIT] Still have the time to respond.
                    return;
                }
            } else if (b_score.rank == EJRank::MISS
                    || b_score.rank == EJRank::AUTO) {      // Starting point was scored MISS or AUTO
                assert(b_score.rank == e_score.rank);       // Starting point and ending points must be equal.
                if (e_temp.rank == EJRank::MISS) {  // [DEAD] Past target time
                    dead = true;
                    return;
                } else {                            // [DONE] Not past target time
                    return;
                }
            } else {                                // Starting point was scored.
                if (e_score.rank == EJRank::NONE) {         // Ending point hasn't, so the player was holding this.
                    if (e_temp.rank == EJRank::NONE) {              // [DONE] Release too early
                        e_score = JScore ( EJRank::MISS, 0, e_temp.delta );
                        calculateScore();
                        return;
                    } else {                                        // [DEAD] Release at the right time
                        e_score = e_temp;
                        calculateScore();
                        return;
                    }
                } else {                                    // Ending point was scored.
                    if (e_temp.rank == EJRank::MISS) {              // [DEAD] Past target time
                        dead = true;
                        return;
                    } else {                                        // [DONE] Not past target time
                        return;
                    }
                }
            }

        case KeyStatus::ON: // Just hit the key or rehit after miss.
            if (b_score.rank == EJRank::NONE) {             // Starting point was not hit
                am->play(b_sample_id, 1, vol, pan);  // Play sound.
                if (b_temp.rank == EJRank::NONE) {                  // [WAIT] Hit too early
                    return;
                } else {                                            // [LIVE] Staring point scores!
                    b_score = b_temp;
                    return;                                     // DO NOT CLEAR FROM ACTIVE QUEUE
                }
            } else if (e_score.rank == EJRank::NONE) {      // Starting point was hit, ending point hasn't
                if (e_temp.rank == EJRank::MISS) {                  // [DEAD] Past target time
                    e_score = e_temp;
                    calculateScore();
                    dead = true;
                    return;
                } else {                                            // [DONE] Not past target time
                    am->play(b_sample_id, 1, vol, pan);             // Play sound.
                    return;
                }
            } else {
                return;
            }

        default:
            return;
    }
}



NoteList zip_key(SingleNoteList H, SingleNoteList R)
{
    NoteList L;

    while((!H.empty()) && (!R.empty()))
    {
        SingleNoteList::iterator
            h = H.begin(), r = R.begin();
        Note_Single
            *hn = *h, *rn = *r;

        if (hn->getTime() > rn->getTime())
            throw std::runtime_error("Bad chart.");

        L.push_back(new Note_Long(*hn, *rn, hn->getVol(), hn->getPan()));
        H.erase(h); delete hn;
        R.erase(r); delete rn;
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


