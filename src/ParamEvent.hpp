//  ParamEvent.h :: Note object
//  Copyright 2013 Keigen Shu

#ifndef PARAM_EVENT_H
#define PARAM_EVENT_H

#include <cstdint>
#include <list>
#include <queue>
#include "Chrono.hpp"

enum class EParam : uint16_t
{
    EP_ACTIVE       = 0x8000, // Triggered?

    /* Clock Parameters */
    EP_C_TEMPO      = 0x0310,
    EP_C_TS_A       = 0x0321, // Time-signature Ticks per Beat
    EP_C_TS_B       = 0x0322, // Time-signature Beats per Measure
    EP_C_TS_Z       = 0x0324, // Floating-point Ticks per Measure

    EP_C_STOP_T     = 0x0325, // Tick-base stop
    EP_C_STOP_R     = 0x0326, // Real-base stop

    /* Modifier Parameters */
    EP_M_SPEED_X    = 0x0D10, // Speed multiplier

    /* Judgement Parameters */


    /* Audio Engine Parameters */
    EP_A_VOL        = 0x0111, // Global volume
    EP_A_PAN        = 0x0112, // Global panning

    /* Video Engine Parameters */


};

struct ParamEvent
{
    TTime  time ;
    EParam param;

    union Value
    {
        double     asFloat;
        int64_t    asInt;

        Value (double  f) : asFloat(f) {}
        Value (int64_t i) : asInt  (i) {}
    } value;

    ParamEvent (const TTime& _time, const EParam& _param, double  _value) : time(_time), param(_param), value(_value) {}
    ParamEvent (const TTime& _time, const EParam& _param, int64_t _value) : time(_time), param(_param), value(_value) {}

    inline void set  () {
        param = static_cast<EParam>(
                static_cast<int16_t>(param) |
                static_cast<int16_t>(EParam::EP_ACTIVE)
                );
    }

    inline void reset() {
        param = static_cast<EParam>(
                static_cast<int16_t>(param) ^
                static_cast<int16_t>(EParam::EP_ACTIVE)
                );
    }
    inline bool test () const { return param > EParam::EP_ACTIVE; }
};

inline bool cmpParamEvent_Greater(const ParamEvent * const &a, const ParamEvent * const &b)
{
    return (a->time == b->time) ? (a->param < b->param) : (a->time < b->time);
}

typedef std::list <ParamEvent*> ParamEventList;
typedef std::queue<ParamEvent*> ParamEventQueue;

#endif
