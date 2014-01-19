//  aweFilter.h :: Filter class
//  Copyright 2013 Keigen Shu

#ifndef AWE_FILTER_H
#define AWE_FILTER_H

#include "aweDefine.h"
#include "aweBuffer.h"

namespace awe {

/* Define namespace for Filters*/
namespace Filter {}

/** Mono/Stereo audio filter interface
 * This class serves as an interface to all audio filtering objects.
 */
class AscFilter
{
public:
    virtual ~AscFilter() { }

    //! Resets the filter as if it has just been created.
    virtual void reset_state() = 0;

    //! Filters input as mono-channeled Afloat sample
    //! @param[in,out] buffer buffer to filter through
    virtual void doBuffer(AfBuffer &buffer) = 0;

};
}
#endif
