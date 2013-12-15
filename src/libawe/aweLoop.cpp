//  aweLoop.cpp :: Looping sequence object
//  Copyright 2012 - 2013 Keigen Shu

#include <stdexcept>
#include <sstream>
#include <bitset>

#include "aweLoop.h"

namespace awe {

Aloop::Aloop(
        const double &_begin,
        const double &_end,
        const Aloop::Mode &_mode,
        const bool _startPaused
        ) :
    mode    (_mode),
    paused  (_startPaused),
    begin   (_begin),
    now     (isReverse(_mode) ? _end : _begin),
    end     (_end)
    { }

Aloop::Aloop(
        const double &_begin,
        const double &_now,
        const double &_end,
        const Aloop::Mode &_mode,
        const bool _startPaused
        ) :
    mode    (_mode),
    paused  (_startPaused),
    begin   (_begin),
    now     (_now),
    end     (_end)
    { }

bool Aloop::operator+= (const double &b)
{
    if (paused) return true;

    switch (getMethod(mode))
    {
        case Aloop::Mode::UNDEFINED:
#ifdef DEBUG
            printf("libawe [warn] Undefined loop mode on object %p.\n", this);
#endif
        case Aloop::Mode::ONCE:
            if (isForward(mode)) {
                now += b;
                if (now > end) {
                    now = end;
                    paused = true;
                }
            } else {
                now -= b;
                if (now < begin) {
                    now = begin;
                    paused = true;
                }
            }
            break;

        case Aloop::Mode::REPEAT:
            if (isForward(mode))
                now += b;
            else
                now -= b;

            if (now > end)
                now = begin;
            else if (now < begin)
                now = end;

            break;

        case Aloop::Mode::ALTERNATING:
            if (isForward(mode))
                now += b;
            else
                now -= b;

            if (now < begin || now > end)
                mode = ~mode;

            break;

        default:
            std::ostringstream error_string;
            error_string
                << "Invalid loop mode {in binary " << std::bitset<8>(*mode)
                << "} encountered on loop object " << std::hex << this
                << "." << std::endl;
            throw std::invalid_argument(error_string.str().c_str());

            return true;
    } // switch(getMethod(mode))

    return false;
}

}
