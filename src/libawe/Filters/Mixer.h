//  Filters/Mixer.h :: Basic mixer filter
//  Copyright 2013 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_FILTER_MIXER_H
#define AWE_FILTER_MIXER_H

#include "../aweFilter.h"

namespace awe {
namespace Filter {

class AscMixer : public AscFilter
{
public:
    //! Channel gain parameter generator using linear panning law.
    static const Asfloatf xLinear(Afloat const &vol, Afloat const &pan);

    //! Channel gain parameter generator using circular panning law.
    static const Asfloatf xSinCos(Afloat const &vol, Afloat const &pan);

private:
    //! Pointer to a channel gain parameter generator
    const Asfloatf (*law)(Afloat const &vol, Afloat const &pan);

    Afloat      vol;    //!< Output volume
    Afloat      pan;    //!< Output panning factor
    Asfloatf    chgain; //!< Calculated gain applied on each channel

public:
    //! Mixer panning law enumerator.
    enum class IEType : std::uint8_t
    {
        LINEAR = static_cast<std::uint8_t>('L'), //!< Linear panning law
        SINCOS = static_cast<std::uint8_t>('C')  //!< Circular panning law
    };

    //! Default constructor.
    AscMixer(
        Afloat _vol = 1.0f,
        Afloat _pan = 0.0f,
        IEType type = IEType::SINCOS
    )   : law(type == IEType::LINEAR ? &xLinear : &xSinCos)
        , vol(_vol)
        , pan(_pan)
        , chgain(law(_vol, _pan))
    { }

    inline void reset(Afloat _vol, Afloat _pan)
    {
        vol     = _vol;
        pan     = _pan;
        chgain  = law(_vol, _pan);
    }

    inline Afloat getVol() const { return vol; }
    inline Afloat getPan() const { return pan; }

    inline void setVol(Afloat _vol) { vol = _vol; reset(_vol, pan); }
    inline void setPan(Afloat _pan) { pan = _pan; reset(vol, _pan); }

    void reset_state() override { reset(vol, pan); }

    void doBuffer(AfBuffer &buffer) override {
        /****/ if (buffer.getChannelCount() == 0) {
            return;
        } else if (buffer.getChannelCount() == 1) {
            std::for_each(
                    buffer.begin(), buffer.end(),
                    [this](Afloat &value) {
                        value *= vol;
                    });
        } else {
            for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
            {
                Afloat* f = buffer.getFrame(i);
                f[0] *= chgain[0];
                f[1] *= chgain[1];
            }
        }

    }

    //!@name Apply-on-sample operations
    //!@{
    inline void doM(Afloat &m) { m *= vol; }
    inline void doL(Afloat &l) { l *= chgain[0]; }
    inline void doR(Afloat &r) { r *= chgain[1]; }

    inline void doM(Aint   &m) { m = to_Aint(to_Afloat(m) * vol); }
    inline void doL(Aint   &l) { l = to_Aint(to_Afloat(l) * chgain[0]); }
    inline void doR(Aint   &r) { r = to_Aint(to_Afloat(r) * chgain[1]); }
    //!@}

    //!@name Apply-on-frame operations
    //!@{
    inline void doF(Asfloatf &f) { f *= chgain; }
    inline void doF(Asintf   &f) {
        f.data[0] = to_Aint(to_Afloat(f.data[0]) * chgain[0]);
        f.data[1] = to_Aint(to_Afloat(f.data[1]) * chgain[1]);
    }
    //!@}

    //!@name Translate-from-sample operations
    //!@{
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
    //!@}
};

}
}
#endif
