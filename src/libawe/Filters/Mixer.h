//  Filters/Mixer.h :: Basic mixer filter
//  Copyright 2013 Keigen Shu

#ifndef AWE_FILTER_MIXER_H
#define AWE_FILTER_MIXER_H

#include "../aweFilter.h"
#include <cassert>

namespace awe {
namespace Filter {

class AscMixer : public AscFilter
{
public:
    static const Asfloatf xLinear(Afloat const &vol, Afloat const &pan);
    static const Asfloatf xSinCos(Afloat const &vol, Afloat const &pan);

private:
    const  Asfloatf (*law)(Afloat const &vol, Afloat const &pan);

    Afloat      vol;    // Volume
    Afloat      pan;    // Panning factor
    Asfloatf    chgain; // Calculated gain applied on each channel

public:
    enum class IEType : std::uint8_t
    {
        LINEAR = static_cast<std::uint8_t>('L'),
        SINCOS = static_cast<std::uint8_t>('C')
    };

    AscMixer(
            Afloat _vol = 1.0f,
            Afloat _pan = 0.0f,
            IEType type = IEType::SINCOS
            ) :
        law(type == IEType::LINEAR ? &xLinear : &xSinCos),
        vol(_vol), pan(_pan),
        chgain(law(_vol, _pan))
        {}

    inline void reset_state() {}

    inline void reset(Afloat _vol, Afloat _pan)
    {
        vol = _vol;
        pan = _pan;
        chgain  = law(_vol, _pan);
    }

    inline Afloat getVol() const { return vol; }
    inline Afloat getPan() const { return pan; }

    inline void setVol(Afloat _vol) { vol = _vol; reset(_vol, pan); }
    inline void setPan(Afloat _pan) { pan = _pan; reset(vol, _pan); }

    inline void doM(Afloat &m) { m *= vol; }
    inline void doL(Afloat &l) { l *= chgain[0]; }
    inline void doR(Afloat &r) { r *= chgain[1]; }

    inline void doM(Aint   &m) { m = to_Aint(to_Afloat(m) * vol); }
    inline void doL(Aint   &l) { l = to_Aint(to_Afloat(l) * chgain[0]); }
    inline void doR(Aint   &r) { r = to_Aint(to_Afloat(r) * chgain[1]); }

    inline void doF(Asfloatf &f) { f *= chgain; }
    inline void doF(Asintf   &f) {
        f.data[0] = to_Aint(to_Afloat(f.data[0]) * chgain[0]);
        f.data[1] = to_Aint(to_Afloat(f.data[1]) * chgain[1]);
    }

    inline Afloat   ffdoM(Afloat   const &m) { return m * vol; }
    inline Afloat   ffdoL(Afloat   const &l) { return l * chgain[0]; }
    inline Afloat   ffdoR(Afloat   const &r) { return r * chgain[1]; }

    inline Afloat   ifdoM(Aint     const &m) { return to_Afloat(m) * vol; }
    inline Afloat   ifdoL(Aint     const &l) { return to_Afloat(l) * chgain[0]; }
    inline Afloat   ifdoR(Aint     const &r) { return to_Afloat(r) * chgain[1]; }

    inline Aint     iidoM(Aint     const &m) { return to_Aint(to_Afloat(m) * vol); }
    inline Aint     iidoL(Aint     const &l) { return to_Aint(to_Afloat(l) * chgain[0]); }
    inline Aint     iidoR(Aint     const &r) { return to_Aint(to_Afloat(r) * chgain[1]); }

    inline Aint     fidoM(Afloat   const &m) { return to_Aint(m * chgain[0]); }
    inline Aint     fidoL(Afloat   const &l) { return to_Aint(l * chgain[0]); }
    inline Aint     fidoR(Afloat   const &r) { return to_Aint(r * chgain[1]); }

    inline Asfloatf ffdoF(Asfloatf const &f) { return Asfloatf(f) * chgain; }
    inline Asintf   iidoF(Asintf   const &f) {
        Asintf o;
        o[0] = to_Aint(to_Afloat(f.data[0]) * chgain[0]);
        o[1] = to_Aint(to_Afloat(f.data[1]) * chgain[1]);
        return o;
    }

    template <typename Iterator>
        inline void  doI(
                Iterator begin,
                Iterator end,
                const uint8_t &channels
                ) {
            assert(channels >= 2 && "AscMixer accepting iterator input requires at least two channels per frame.");

            auto iter = begin;

            if (std::numeric_limits< typename std::iterator_traits<Iterator>::value_type >::is_integer)
            {
                for (; iter != end; iter += channels)
                {
                    *(iter)   = to_Aint(to_Afloat(*(iter  )) * chgain[0]);
                    *(iter+1) = to_Aint(to_Afloat(*(iter+1)) * chgain[1]);
                }
            } else {
                for (; iter != end; iter += channels)
                {
                    *(iter)   *= chgain[0];
                    *(iter+1) *= chgain[1];
                }
            }
        }

    template <typename InputIterator, typename OutputIterator>
        inline void cdoI(
                InputIterator sbegin,
                InputIterator send,
                OutputIterator dbegin,
                const uint8_t &schannels,
                const uint8_t &dchannels
                ) {
            assert(schannels >= 2 && "AschMixer accepting iterator input requires at least two channels per frame.");
            assert(dchannels >= 2 && "Iterator accepting AscMixer output requires at least two channels per frame.");

            auto siter = sbegin;
            auto diter = dbegin;

            if (
                    std::is_integral< typename std::iterator_traits<InputIterator>::value_type >::value &&
                    std::is_integral< typename std::iterator_traits<OutputIterator>::value_type >::value
               )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = to_Aint(to_Afloat(*(siter  )) * chgain[0]);
                    *(diter+1) = to_Aint(to_Afloat(*(siter+1)) * chgain[1]);
                }
            } else if (
                    std::is_integral      < typename std::iterator_traits<InputIterator>::value_type >::value &&
                    std::is_floating_point< typename std::iterator_traits<OutputIterator>::value_type >::value
                    )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = to_Afloat(*(siter  )) * chgain[0];
                    *(diter+1) = to_Afloat(*(siter+1)) * chgain[1];
                }
            } else if (
                    std::is_floating_point< typename std::iterator_traits<InputIterator>::value_type >::value &&
                    std::is_integral      < typename std::iterator_traits<OutputIterator>::value_type >::value
                    )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = to_Aint(*(siter  ) * chgain[0]);
                    *(diter+1) = to_Aint(*(siter+1) * chgain[1]);
                }
            } else {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = *(siter)   * chgain[0];
                    *(diter+1) = *(siter+1) * chgain[1];
                }
            }
        }
};

}
}
#endif
