//  aweFilter.h :: Filter class
//  Copyright 2013 Keigen Shu

#ifndef AWE_FILTER_H
#define AWE_FILTER_H

#include "aweDefine.h"

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

    /**
     * Resets the filter as if it has just been created.
     */
    virtual void reset_state() = 0;

    //! Filters input as mono-channeled Afloat sample
    //! @param[in,out] m sample to filter
    virtual void doM(Afloat &m) = 0;
    //! Filters input as left-channel Afloat sample
    //! @param[in,out] l sample to filter
    virtual void doL(Afloat &l) = 0;
    //! Filters input as right-channel Afloat sample
    //! @param[in,out] r sample to filter
    virtual void doR(Afloat &r) = 0;

    //! Filters input as mono-channeled Aint sample
    //! @param[in,out] m sample to filter
    virtual void doM(Aint &m) = 0;
    //! Filters input as left-channel Aint sample
    //! @param[in,out] l sample to filter
    virtual void doL(Aint &l) = 0;
    //! Filters input as right-channel Aint sample
    //! @param[in,out] r sample to filter
    virtual void doR(Aint &r) = 0;

    //! Filters input stereo Afloat frame
    //! @param[in,out] f frame to filter
    virtual void doF(Asfloatf &f) = 0;
    //! Filters input stereo Aint frame
    //! @param[in,out] f frame to filter
    virtual void doF(Asintf   &f) = 0;

    //! Returns filtered Afloat copy of input mono-channeled Afloat sample
    //! @param[in] m sample to filter
    virtual Afloat ffdoM(Afloat const &m) = 0;
    //! Returns filtered Afloat copy of input left-channel Afloat sample
    //! @param[in] l sample to filter
    virtual Afloat ffdoL(Afloat const &l) = 0;
    //! Returns filtered Afloat copy of input right-channel Afloat sample
    //! @param[in] r sample to filter
    virtual Afloat ffdoR(Afloat const &r) = 0;

    //! Returns filtered Afloat copy of input mono-channeled Aint sample
    //! @param[in] m sample to filter
    virtual Afloat ifdoM(Aint const &m) = 0;
    //! Returns filtered Afloat copy of input left-channel Aint sample
    //! @param[in] l sample to filter
    virtual Afloat ifdoL(Aint const &l) = 0;
    //! Returns filtered Afloat copy of input right-channel Aint sample
    //! @param[in] r sample to filter
    virtual Afloat ifdoR(Aint const &r) = 0;

    //! Returns filtered Aint copy of input mono-channeled Aint sample
    //! @param[in] m sample to filter
    virtual Aint iidoM(Aint const &m) = 0;
    //! Returns filtered Aint copy of input left-channel Aint sample
    //! @param[in] l sample to filter
    virtual Aint iidoL(Aint const &l) = 0;
    //! Returns filtered Aint copy of input right-channel Aint sample
    //! @param[in] r sample to filter
    virtual Aint iidoR(Aint const &r) = 0;

    //! Returns filtered Aint copy of input mono-channeled Afloat sample
    //! @param[in] m sample to filter
    virtual Aint fidoM(Afloat const &m) = 0;
    //! Returns filtered Aint copy of input left-channel Afloat sample
    //! @param[in] l sample to filter
    virtual Aint fidoL(Afloat const &l) = 0;
    //! Returns filtered Aint copy of input right-channel Afloat sample
    //! @param[in] r sample to filter
    virtual Aint fidoR(Afloat const &r) = 0;

    //! Returns filtered copy of input stereo Afloat frame
    //! @param[in] f frame to filter
    virtual Asfloatf ffdoF(Asfloatf const &f) = 0;
    //! Returns filtered copy of input stereo Aint frame
    //! @param[in] f frame to filter
    virtual Asintf   iidoF(Asintf   const &f) = 0;
};
}
#endif
