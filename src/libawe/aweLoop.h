//  aweLoop.h :: Looping sequence object
//  Copyright 2012 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_LOOP_H
#define AWE_LOOP_H

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace awe {

/*! Class managing sequence progression.
 */
class Aloop
{
public:
    /*! Loop operation mode enumerator.
     *  
     *  The first two least significant bits is the loop traversal method.
     *  The third bit is not used but reserved for future additions.
     *  The forth bit is the current loop movement direction.
     */
    enum class Mode : std::uint8_t {
        UNDEFINED   = 0x0, //!< Unused and treated the same as `ONCE`.

        ONCE        = 0x1, //!< Traverse slice from beginning to end only once.
        REPEAT      = 0x2, //!< Traverse slice from beginning to end and repeat forever.
        ALTERNATING = 0x3, //!< Traverse slice from beginning to end then back and repeat forever.

        FORWARD     = 0x0, //!< Traverse in normal direction (begin to end).
        REVERSE     = 0x8, //!< Traverse in reverse direction (end to begin).

        /** Default loop mode: `[ FORWARD | ONCE ]` */
        __DEFAULT   = FORWARD | ONCE,

        __LOOP_MODE = 0x3, //!< Loop mode bit mask.
        __DIRECTION = 0x8, //!< Loop direction bit mask.
    };

    Mode   mode;    //!< Looping mode.
    bool   paused;  //!< Stops movement if set to true.
    double begin;   //!< The beginning offset of the loop.
    double now;     //!< The current offset of the loop.
    double end;     //!< The ending offset of the loop.

    Aloop(const double &_begin, const double &_end, const Aloop::Mode &_mode, const bool startPaused = false);
    Aloop(const double &_begin, const double &_end, const double &_now, const Aloop::Mode &_mode, const bool startPaused = false);

    /*! Moves the current position by `b`.
     *  @return true if the loop is/has been paused.
     */
    bool operator+= (const double &b);

    /*! Moves the current position by `-b`.
     *  @return true if the loop is/has been paused.
     */
    inline bool operator-= (const double &b) { return operator+=(-b); }

    /*! Returns the starting position of the loop sequence.
     *  @return Position of the beginning of the loop sequence.
     */
    inline unsigned long ubegin () const { return std::floor(begin); }

    /*! Returns the ending position of the loop sequence.
     *  @return Position of the end of the loop sequence.
     */
    inline unsigned long uend   () const { return std::floor(end); }

    /*! Returns the current position of the loop sequence.
     *  @return Current position of the loop sequence.
     */
    inline unsigned long unow   () const {
        //# :: DO NOT TOUCH ::
        //# THIS MESS FIXES ANNOYING PULSES AT END OF SOUND RENDER
        return std::min<unsigned long>(now, uend() - 1);
    }

    /*! Returns the current position relative to the end of the loop sequence.
     *  @return Current position relative to end of loop.
     */
    inline double getPosition() const { return  now / end; }

    /*! Returns the current position relative to the range of the loop.
     *  @return Current position relative to traversal range of loop.
     */
    inline double getProgress() const { return (now - begin) / (end - begin); }
};

//!@name Global Methods and Operator Overrides for Aloop::Mode
//!@{

inline static std::uint8_t operator* (const Aloop::Mode&  mode) { return static_cast<std::uint8_t>(mode); }
inline static std::uint8_t operator* (const Aloop::Mode&& mode) { return static_cast<std::uint8_t>(mode); }

inline static bool        isReverse (const Aloop::Mode &mode) { return (*mode & *Aloop::Mode::__DIRECTION) == *Aloop::Mode::__DIRECTION; }
inline static bool        isForward (const Aloop::Mode &mode) { return !isReverse(mode); }

/*! Retrieves the loop traversal method segment of the Mode enumerator.
 *  @see Aloop::Mode Enumerator definition.
 */
inline static Aloop::Mode getMethod (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode & *Aloop::Mode::__LOOP_MODE); } /* b'00000111' */

inline static Aloop::Mode operator& (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a & *b); } //!< Bit-wise AND operator
inline static Aloop::Mode operator| (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a | *b); } //!< Bit-wise  OR operator
inline static Aloop::Mode operator^ (const Aloop::Mode &a, const Aloop::Mode &b) { return static_cast<Aloop::Mode>(*a ^ *b); } //!< Bit-wise XOR operator

//! Inverts the loop direction
inline static Aloop::Mode operator~ (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode ^  *Aloop::Mode::__DIRECTION); }

//! Forces the direction to forward
inline static Aloop::Mode operator+ (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode & ~*Aloop::Mode::__DIRECTION); /* b'11110111' */ }

//! Forces the direction to reverse
inline static Aloop::Mode operator- (const Aloop::Mode &mode) { return static_cast<Aloop::Mode>(*mode |  *Aloop::Mode::__DIRECTION); /* b'00001000' */ }

//!@}

}
#endif
