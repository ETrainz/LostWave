//  Filters/Rack.cpp :: Mixer filter rack
//  Copyright 2013 Keigen Shu

#include "Mixer.h"

namespace awe {
namespace Filter {

const Asfloatf AscMixer::xLinear(Afloat const &vol, Afloat const &pan)
{
    const Afloat l = (1.0f + pan) / 2.0f;
    Asfloatf f;
    f[0] = 2.0f * l * vol;
    f[1] = 2.0f * (1.0f - l) * vol;
    return f;
}

const Asfloatf AscMixer::xSinCos(Afloat const &vol, Afloat const &pan)
{
    const Afloat p = M_PI * (1.0f + pan) / 4.0f;
    Asfloatf f;
    f[0] = (float)cos(p) * vol;
    f[1] = (float)sin(p) * vol;
    return f;
}

}
}
