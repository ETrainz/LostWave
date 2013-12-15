//  aweSource.h :: Sound source base class
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_SOURCE_H
#define AWE_SOURCE_H

#include <set>
#include "aweBuffer.h"

namespace awe {

class Asource
{
public:
    virtual ~Asource() {}

    /**
     * Make this thing into an active sound source.
     */
    virtual void make_active (void* userData = nullptr) = 0;

    /**
     * Is this thing activated?
     */
    virtual bool is_active () const = 0;

    /**
     * Render function used by the Atrack class.
     */
    virtual void render (AfBuffer &targetBuffer, const ArenderConfig &targetConfig) = 0;

    /**
     * Called when the source is being released by a manager.
     */
    virtual void drop () = 0;
};

typedef std::set<Asource*> AsourceSet;
}

#endif
