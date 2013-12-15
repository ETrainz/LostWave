/* Filters/3BEQ.h :: 3-band equalizer
 * Taken from http://www.musicdsp.org/archive.php?classid=3#236
 *
 * Copyright 2006 Neil C. / Etanza Systems
 *
 * This file is hereby placed under the public domain for all purposes,
 * including use in commercial applications.
 *
 * The author assumes NO RESPONSIBILITY for any of the problems caused
 * by the use of this software.
 */
#ifndef AWE_FILTER_3BAND_EQ_H
#define AWE_FILTER_3BAND_EQ_H

#include <cassert>
#include "../aweFilter.h"

namespace awe {
namespace Filter {

class Asc3BEQ : public AscFilter
{
private:
    double lf, hf; // Low-band and high-band frequencies
    double lg, mg, hg; // Low-band, mid-band and high-band gain

    double llp[4], lhp[4]; // Low-band and high-band poles -- left
    double rlp[4], rhp[4]; // Low-band and high-band poles -- right
    double la [3], ra [3]; // Left and right sample accumulator

    double const d = 1.0 / 4294967295.0; // Denormalization constant
public:
    Asc3BEQ(
            double mixfreq,
            double lo_freq = 880.0,
            double hi_freq = 5000.0,
            double lo_gain = 1.0,
            double mi_gain = 1.0,
            double hi_gain = 1.0
           );

    inline void reset_state()
    {
        llp[0] = llp[1] = llp[2] = llp[3] = 0.0;
        lhp[0] = lhp[1] = lhp[2] = lhp[3] = 0.0;
        rlp[0] = rlp[1] = rlp[2] = rlp[3] = 0.0;
        rhp[0] = rhp[1] = rhp[2] = rhp[3] = 0.0;
        la[0] = la[1] = la[2] = 0.0;
        ra[0] = ra[1] = ra[2] = 0.0;
    }

    inline void doM(Afloat &m) { m = ffdoM(m); }
    inline void doL(Afloat &l) { l = ffdoL(l); }
    inline void doR(Afloat &r) { r = ffdoR(r); }

    inline void doM(Aint &m) { m = iidoM(m); }
    inline void doL(Aint &l) { l = iidoL(l); }
    inline void doR(Aint &r) { r = iidoR(r); }

    inline void doF(Asfloatf &f) { doL(f[0]); doR(f[1]); }
    inline void doF(Asintf   &f) { doL(f[0]); doR(f[1]); }

    inline Afloat ffdoM(Afloat const &m) { return (ffdoL(m) + ffdoR(m)) / 2.0f;}

    Afloat ffdoL(Afloat const &l);
    Afloat ffdoR(Afloat const &r);

    inline Afloat ifdoM(Aint const &m) { return ffdoM(to_Afloat(m)); }
    inline Afloat ifdoL(Aint const &l) { return ffdoL(to_Afloat(l)); }
    inline Afloat ifdoR(Aint const &r) { return ffdoR(to_Afloat(r)); }

    inline Aint iidoM(Aint const &m) { return to_Aint(ffdoM(to_Afloat(m))); }
    inline Aint iidoL(Aint const &l) { return to_Aint(ffdoL(to_Afloat(l))); }
    inline Aint iidoR(Aint const &r) { return to_Aint(ffdoR(to_Afloat(r))); }

    inline Aint fidoM(Afloat const &m) { return to_Aint(ffdoM(m)); }
    inline Aint fidoL(Afloat const &l) { return to_Aint(ffdoL(l)); }
    inline Aint fidoR(Afloat const &r) { return to_Aint(ffdoR(r)); }

    inline Asfloatf ffdoF(Asfloatf const &f) {
        Asfloatf o;
        o[0] = ffdoL(f.data[0]);
        o[1] = ffdoL(f.data[1]);
        return o;
    }

    inline Asintf iidoF(Asintf const &f) {
        Asintf o;
        o[0] = iidoL(f.data[0]);
        o[1] = iidoR(f.data[1]);
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
                    *(iter)   = iidoL(*(iter  ));
                    *(iter+1) = iidoR(*(iter+1));
                }
            } else {
                for (; iter != end; iter += channels)
                {
                    *(iter)   = ffdoL(*(iter  ));
                    *(iter+1) = ffdoR(*(iter+1));
                }
            }
        }

    template <typename InputIt, typename OutputIt>
        inline void cdoI(
                InputIt sbegin,
                InputIt send,
                OutputIt dbegin,
                const uint8_t &schannels,
                const uint8_t &dchannels
                ) {
            assert(schannels >= 2 && "AschMixer accepting iterator input requires at least two channels per frame.");
            assert(dchannels >= 2 && "Iterator accepting AscMixer output requires at least two channels per frame.");

            auto siter = sbegin;
            auto diter = dbegin;

            if (
                    std::is_integral< typename std::iterator_traits<InputIt>::value_type >::value &&
                    std::is_integral< typename std::iterator_traits<OutputIt>::value_type >::value
               )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = iidoL(*(siter  ));
                    *(diter+1) = iidoR(*(siter+1));
                }
            } else if (
                    std::is_integral      < typename std::iterator_traits<InputIt>::value_type >::value &&
                    std::is_floating_point< typename std::iterator_traits<OutputIt>::value_type >::value
                    )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = ifdoL(*(siter  ));
                    *(diter+1) = ifdoR(*(siter+1));
                }
            } else if (
                    std::is_floating_point< typename std::iterator_traits<InputIt>::value_type >::value &&
                    std::is_integral      < typename std::iterator_traits<OutputIt>::value_type >::value
                    )
            {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = fidoL(*(siter  ));
                    *(diter+1) = fidoR(*(siter+1));
                }
            } else {
                for (; siter != send; siter += schannels, diter += dchannels)
                {
                    *(diter)   = ffdoL(*(siter  ));
                    *(diter+1) = ffdoR(*(siter+1));
                }
            }
        }

};

}
}

#endif
