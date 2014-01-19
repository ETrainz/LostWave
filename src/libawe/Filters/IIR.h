//  Filters/IIR.h :: Infinite Impulse Response Filter
//  Copyright (c) 2014 Keigen Shu

#ifndef AWE_FILTER_IIR_H
#define AWE_FILTER_IIR_H

#include "../aweFilter.h"
#include <array>

namespace awe {
namespace Filter {

/**
 * 2nd-order IIR filter.
 *
 * Filtering is performed using direct transpose canonical realization.
 *
 * Based on the book 'Digital Filter Design' by Zoran Milivojevic.
 *
 *      http://www.mikroe.com/products/view/268/digital-filter-design/
 */
class AscIIR
{
private:
    std::array<double, 3> mB , mA ; // B and A coefficients (mA[0] is unused)
    std::array<double, 2> mZl, mZr; // Z delay line for left and right channels

public:
    AscIIR(std::array<double, 6> coefficients) noexcept;

    /** Constructs a Low Pass Filter.
     */
    static std::array<double, 6> newLPF(const double rate, const double freq) noexcept;

    /** Constructs a High Pass Filter.
     */
    static std::array<double, 6> newHPF(const double rate, const double freq) noexcept;

    /** Resets the filter's processing state.
     */
    inline void
        reset() noexcept
        {
            for(double& x : mZl) x = 0;
            for(double& x : mZr) x = 0;
        }

    static void
        process(const std::array<double, 3>& b,
                const std::array<double, 3>& a,
                std::array<double, 2>& z,
                double& x
                ) noexcept;

    inline void
        processL(double& l) noexcept { process(mB, mA, mZl, l); }
    inline void
        processR(double& r) noexcept { process(mB, mA, mZr, r); }

    /** Performs the filtering operation on an entire buffer.
     */
    inline void
        process(AfBuffer& buffer) noexcept
        {
            for(size_t i = 0; i < buffer.getFrameCount(); i += 1)
            {
                Afloat* x = buffer.getFrame(i);

                double l = x[0], r = x[1];

                processL(l);
                processR(r);

                x[0] = l, x[1] = r;
            }
        }


};

}
}

#endif
