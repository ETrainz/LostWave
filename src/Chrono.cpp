#include "Chrono.hpp"

bool TTime::operator== (const TTime &cmpTTime) const
{
    if (tick == cmpTTime.tick && beat == cmpTTime.beat && measure == cmpTTime.measure)
        return true;
    return false;
}
bool TTime::operator>= (const TTime &cmpTTime) const
{
    if (tick >= cmpTTime.tick && (beat == cmpTTime.beat && measure == cmpTTime.measure))
        return true;
    else if ((beat > cmpTTime.beat && measure == cmpTTime.measure) || measure > cmpTTime.measure)
        return true;
    else
        return false;
}
bool TTime::operator<= (const TTime &cmpTTime) const
{
    if (tick <= cmpTTime.tick && (beat == cmpTTime.beat && measure == cmpTTime.measure))
        return true;
    else if ((beat < cmpTTime.beat && measure == cmpTTime.measure) || measure < cmpTTime.measure)
        return true;
    else
        return false;
}
bool TTime::operator> (const TTime &cmpTTime) const
{
    if (tick > cmpTTime.tick && (beat == cmpTTime.beat && measure == cmpTTime.measure))
        return true;
    else if ((beat > cmpTTime.beat && measure == cmpTTime.measure) || measure > cmpTTime.measure)
        return true;
    else
        return false;
}
bool TTime::operator< (const TTime &cmpTTime) const
{
    if (tick < cmpTTime.tick && (beat == cmpTTime.beat && measure == cmpTTime.measure))
        return true;
    else if ((beat < cmpTTime.beat && measure == cmpTTime.measure) || measure < cmpTTime.measure)
        return true;
    else
        return false;
}




// Resets music time point
void TClock::resetClock (double BPM, bool startNow)
{
    tsg_tpb = 48;
    tsg_bpm = 4;
    tsg_tpm = tsg_tpb * tsg_bpm;

    tsg_tempo = 60000.0 / tsg_tpb;

    tmp_bpm  = (BPM < 0.0) ? 120.0 : BPM;
    tmp_mspt = tsg_tempo / tmp_bpm;

    isTicking = startNow;

    tct_tick = 0;
    tct_stop = 0;

    currTTime.reset();
    nextTTime.reset();

    tct_mstt = tmp_mspt;

    tpt_Music = tpt_LastRun = tpt_Segment = sysClock::now();
}

// Update clock. Returns false if interrupted.
bool TClock::update ()
{
    if (isTicking) {
        const sysTimeP tpt_now = sysClock::now();

        // This casting and converting hack is done to make the clock run correctly on Linux.
        // The original works just fine on Windows.
        //       -= std::chrono::duration_cast<TimeUnit>(tpt_now - tpt_LastRun).count();
        tct_mstt -= std::chrono::duration_cast<std::chrono::microseconds>(tpt_now - tpt_LastRun).count() / 1000.0;
        tpt_LastRun = tpt_now;
    }

    while (tct_stop > 0)
    {
        if (tct_mstt < tmp_mspt) {
            tct_mstt += tmp_mspt;
            tct_stop -= 1;
        } else {
            break;
        }
    }

    while (tct_mstt < tmp_mspt)
    {
        // increment tick
        currTTime.tick++;
        tct_tick++;
        tct_mstt += tmp_mspt;

        // Process beat and measure
        if (currTTime.tick >= tsg_tpb) {
            currTTime.tick -= tsg_tpb;
            currTTime.beat++;
        }

        if (currTTime.beat >= tsg_bpm) {
            currTTime.beat -= tsg_bpm;
            currTTime.measure++;
        }

        // Interrupted. Exit now.
        if (currTTime == nextTTime) {
            tpt_Segment = sysClock::now();

            nextTTime.reset();
            return false;
        }
    }

    return true;
}
