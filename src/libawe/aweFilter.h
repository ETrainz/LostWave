//  aweFilter.h :: Filter class
//  Copyright 2013 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_FILTER_H
#define AWE_FILTER_H

#include "aweDefine.h"
#include "aweBuffer.h"

namespace awe {

/* Define namespace for filters */
namespace Filter {}

/*! Stereo audio filter interface.
 *  This class serves as an interface to all audio filtering objects.
 */
class AscFilter
{
public:
    //! Virtual destructor
    virtual ~AscFilter() { }

    //! Resets the filter to its initial state.
    virtual void reset_state() = 0;

    /*! Filters input as stereo-channeled Afloat sample buffer.
     *  @param[in,out] buffer buffer to filter through
     */
    virtual void doBuffer(AfBuffer &buffer) = 0;

};
}
#endif
