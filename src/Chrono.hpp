//  Chrono.hpp :: Tick-based clock objects and declarations
//  Copyright 2011 - 2013 Keigen Shu

#ifndef CHRONO_H
#define CHRONO_H

#include <cstdio>
#include <chrono>

typedef std::chrono::system_clock           sysClock; // Steady clock
typedef std::chrono::time_point<sysClock>   sysTimeP; // Time point from steady clock
typedef std::chrono::milliseconds           TimeUnit; // Time point unit in milliseconds

struct TTime
{
    unsigned tick; // Tick
    unsigned beat; // Beat
    unsigned measure; // Measure

    /* constexpr */ TTime(unsigned _tick = 0, unsigned _beat = 0, unsigned _measure = 0) : tick(_tick), beat(_beat), measure(_measure) { }

    bool operator== (const TTime &cmpTTime) const;
    bool operator>= (const TTime &cmpTTime) const;
    bool operator<= (const TTime &cmpTTime) const;
    bool operator> (const TTime &cmpTTime) const;
    bool operator< (const TTime &cmpTTime) const;

    inline void reset() { tick = beat = measure = 0; }
    inline void print() const { printf("%u:%u:%u", measure, beat, tick); }
};

class TClock
{
private:
    sysTimeP    tpt_Create;     // Clock creation time stamp; updated on every boot.
    sysTimeP    tpt_Music;      // Music start time stamp; updated on every music play.
    sysTimeP    tpt_LastRun;    // Last update time stamp; updated on every game tick.
    sysTimeP    tpt_Segment;    // Current segment time stamp; updated when there is a STOP or BPM change.

    unsigned    tsg_tpb;        // Tick per beat
    unsigned    tsg_bpm;        // Beat per measure
    unsigned    tsg_tpm;        // Tick per measure
    double      tsg_tempo;      // Signature pacing to tempo ratio

    double      tmp_bpm;        // Tempo in beats per minute
    double      tmp_mspt;       // Tempo in milliseconds per tick

    double      tct_mstt;       // Milliseconds left to next tick
    unsigned    tct_tick;       // Total tick count

    unsigned    tct_stop;       // Stop tick count

    bool        isTicking;      // Is the clock ticking?

    TTime       currTTime;      // Current TTime
    TTime       nextTTime;      // Next TTime interrupt
public:
    TClock (double BPM = 0.0, bool startNow = false)
    {
        tpt_Create = tpt_Music = tpt_LastRun = tpt_Segment = sysClock::now();
        this->resetClock(BPM, startNow);
    }

    // Resets music time point
    void resetClock (double BPM = 0.0, bool startNow = false);

    // Update clock. Returns false if interrupted.
    bool update ();

    // Change time signature
    inline void setTCSig (unsigned nA = 4, unsigned nB = 48)
    {
        tsg_tpb = nB;
        tsg_bpm = nA;
        tsg_tpm = tsg_tpb * tsg_bpm;

        // The Z to AB converter automatically assumes 48 ticks as standard,
        // so we don't have to recalculate this.
        // tsg_tempo = 60000.0 / tsg_tpb;
        // This means that tmp_mspt never changes.
        // tmp_mspt = tsg_tempo / tmp_bpm;
    }

    // Change tempo
    inline void setTempo (double BPM)
    {
        printf("[%lf] GOT BPM %lf @ ", tmp_bpm, BPM); currTTime.print(); printf("\n");
        tct_mstt -= tmp_mspt;
        tmp_bpm   = BPM;
        tmp_mspt  = tsg_tempo / tmp_bpm;
        tct_mstt += tmp_mspt;
    }

    inline void setTStop (unsigned t)
    {
        printf("[%lf] GOT TSTOP %u @ ", tmp_bpm, t); currTTime.print(); printf("\n");
        tct_stop = t;
    }
    inline bool isTStopped () const {
        return tct_stop > 0;
    }

    inline unsigned const &getTicksPerBeat   () const { return tsg_tpb; }
    inline unsigned const &getBeatsPerMeasure() const { return tsg_bpm; }
    inline unsigned getTicksPerMeasure() const { return tsg_tpb * tsg_bpm; }

    inline unsigned getTicksSinceMeasure() const { return currTTime.tick + currTTime.beat * tsg_tpb; }

    inline bool status () { return isTicking; }
    inline void start () { isTicking = true ; tct_mstt = 0; }
    inline void pause () { isTicking = false; }
    inline void unpause() { isTicking = true; }

    inline const TTime& cgetTTime() const { return currTTime; }
    inline const TTime& cgetITime() const { return nextTTime; }
    inline TTime& getTTime () { return currTTime; }
    inline TTime& getITime () { return nextTTime; }
    inline void   setITime (const TTime &ITime) { nextTTime = TTime(ITime); }
    inline unsigned getCount () const { return tct_tick; }

    inline unsigned long long getLastRunStamp   () { return std::chrono::duration_cast<TimeUnit>(tpt_LastRun.time_since_epoch()).count(); }
    inline unsigned long long getCreatTimeStamp () { return std::chrono::duration_cast<TimeUnit>(tpt_Create .time_since_epoch()).count(); }
    inline unsigned long long getMusicTimeStamp () { return std::chrono::duration_cast<TimeUnit>(tpt_Music  .time_since_epoch()).count(); }
    inline unsigned long long getSystemTime () { return std::chrono::duration_cast<TimeUnit>(sysClock::now().time_since_epoch()).count(); }

    inline double getTempo_bpm  () { return tmp_bpm ; }
    inline double getTempo_mspt () { return tmp_mspt; }
};


#endif
