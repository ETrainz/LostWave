//  aweLoop.h :: Looping sequence object
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_LOOP_H
#define AWE_LOOP_H

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace awe {

/** Class managing sequence progression.
 */
class Aloop
{
public:
    /**
     * Loop operation mode enumerator
     */
    enum class Mode : std::uint8_t {
        // First two bits is the loop method
        UNDEFINED   = 0x0,

        ONCE        = 0x1,  //! Traverse slice from beginning to end only once
        REPEAT      = 0x2,  //! Traverse slice from beginning to end and repeat forever
        ALTERNATING = 0x3,  //! Traverse slice from beginning to end then back and repeat forever

        // Third bit is unused/reserved
        // Forth bit is move direction
        FORWARD     = 0x0,  //! Traverse in normal direction (begin to end)
        REVERSE     = 0x8,  //! Traverse in reverse direction (end to begin)

        // Default
        __DEFAULT   = FORWARD | ONCE,   //! Default mode setting

        // Masks
        __LOOP_MODE = 0x3,  //! Loop mode bit mask
        __DIRECTION = 0x8,  //! Loop direction bit mask
    };

    Mode   mode;
    bool   paused;
    double begin, now, end;

    Aloop(const double &_begin, const double &_end, const Aloop::Mode &_mode, const bool startPaused = false);
    Aloop(const double &_begin, const double &_end, const double &_now, const Aloop::Mode &_mode, const bool startPaused = false);

    /**
     * Move current position by 'b'.
     * @return true if the loop is/has been paused.
     */
    bool operator+= (const double &b);

    /**
     * Move current position by '-b'.
     * @return true if the loop is/has been paused.
     */
    inline bool operator-= (const double &b) { return operator+=(-b); }

    /** @return Position of the beginning of the loop. */
    inline unsigned long ubegin () const { return std::floor(begin); }

    /** @return Position of the end of the loop. */
    inline unsigned long uend   () const { return std::floor(end); }

    /** @return Current position of the loop traversal. */
    inline unsigned long unow   () const {
        //# :: DO NOT TOUCH ::
        //# THIS MESS FIXES ANNOYING PULSES AT END OF SOUND RENDER
        return std::min<unsigned long>(now, uend() - 1);
    }

    /** @return Current position relative to end of loop. */
    inline double getPosition() const { return  now / end; }

    /** @return Current position relative to traversal range of loop. */
    inline double getProgress() const { return (now - begin) / (end - begin); }
};

inline static std::uint8_t operator* (const Aloop::Mode&  mode) { return static_cast<std::uint8_t>(mode); }
inline static std::uint8_t operator* (const Aloop::Mode&& mode) { return static_cast<std::uint8_t>(mode); }

inline static bool        isReverse (const Aloop::Mode &mode) { return (*mode & *Aloop::Mode::__DIRECTION) == *Aloop::Mode::__DIRECTION; }
inline static bool        isForward (const Aloop::Mode &mode) { return !isReverse(mode); }
inline static Aloop::Mode getMethod (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode & *Aloop::Mode::__LOOP_MODE); } /** b'00000111' */

/* Aloop::Mode global operator overloading. */
inline static Aloop::Mode operator& (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a & *b); } /** Bit-wise AND operator */
inline static Aloop::Mode operator| (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a | *b); } /** Bit-wise  OR operator */
inline static Aloop::Mode operator^ (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a ^ *b); } /** Bit-wise XOR operator */
inline static Aloop::Mode operator~ (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode ^  *Aloop::Mode::__DIRECTION); } /** Inverts the loop direction */
inline static Aloop::Mode operator+ (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode & ~*Aloop::Mode::__DIRECTION); /* b'11110111' */ } /** Forces the direction to forward */
inline static Aloop::Mode operator- (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode |  *Aloop::Mode::__DIRECTION); /* b'00001000' */ } /** Forces the direction to reverse */

}
#endif
