//  Filters/IIR.cpp :: Infinite Impulse Response Filter
//  Copyright (c) 2014 Keigen Shu

#include "IIR.h"

namespace awe {
namespace Filter {


AscIIR::AscIIR(std::array<double, 6> k) noexcept
    : mB ({k[0], k[1], k[2]})
    , mA ({k[3], k[4], k[5]})
    , mZl({0,0})
    , mZr({0,0})
{}

std::array<double, 6> AscIIR::newLPF(const double rate, const double freq) noexcept
{
    std::array<double, 6> k;

    const double n = 1.0 / tan(M_PI * freq / rate);
    const double c = 1.0 / (1.0 + sqrt(2.0)*n + n*n);

    k[0] = c;
    k[1] = c * 2.0;
    k[2] = c;
    k[3] = 1.0;
    k[4] = c * 2.0 * (1.0 - n*n);
    k[5] = c *(1.0 - sqrt(2.0)*n + n*n);

    k[0] /= k[3];
    k[1] /= k[3];
    k[2] /= k[3];
    k[4] /= k[3];
    k[5] /= k[3];

    for(double& x : k)
        if (!(x == x))
            x = 0;

    return k;
}

std::array<double, 6> AscIIR::newHPF(const double rate, const double freq) noexcept
{
    std::array<double, 6> k;

    const double n = tan(M_PI * freq / rate);
    const double c = 1.0 / (1.0 + sqrt(2.0)*n + n*n);

    k[0] = c;
    k[1] = c * -2.0;
    k[2] = c;
    k[3] = 1.0;
    k[4] = c *  2.0 * (n*n - 1.0);
    k[5] = c * (1.0 - sqrt(2.0)*n + n*n);

    k[0] /= k[3];
    k[1] /= k[3];
    k[2] /= k[3];
    k[4] /= k[3];
    k[5] /= k[3];

    for(double& x : k)
        if (!(x == x))
            x = 0;

    return k;
}

void AscIIR::process(
        const std::array<double, 3>& b,
        const std::array<double, 3>& a,
        std::array<double, 2>& z,
        double& x
        ) noexcept
{
    const double  y = x * b[0] + z[0];
    z[0] = x * b[1] - y * a[1] + z[1];
    z[1] = x * b[2] - y * a[2];

    //  Clip very small values ( < -240dB ) to 0
    if (z[0] > -1.0e-12 && z[0] < 1.0e-12)
        z[0] = 0;

    if (z[1] > -1.0e-12 && z[1] < 1.0e-12)
        z[1] = 0;

    x = y;
}

}
}
