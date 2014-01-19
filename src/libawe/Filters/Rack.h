//  Filters/Rack.h :: Mixer filter rack
//  Copyright 2013 Keigen Shu

#ifndef AWE_FILTER_RACK_H
#define AWE_FILTER_RACK_H

#include <cassert>
#include <cstdint>
#include "../aweFilter.h"

namespace awe {
namespace Filter {

class AscRack : public AscFilter
{
private:
    std::vector<AscFilter*> filters;

public:
    AscRack() {}

    inline void reset_state() { for(AscFilter* filter : filters) filter->reset_state(); }
    inline void attach_filter(AscFilter* filter) { filters.push_back(filter); }
    inline void detach_filter(size_t     filter)
    {
        auto it = filters.begin();
        while (it != filters.end() || --filter > 0)
            it++;
        filters.erase(it);
    }

    inline AscFilter       *  getFilter(size_t filter)       { return filters[filter]; }
    inline AscFilter const * cgetFilter(size_t filter) const { return filters[filter]; }

    inline void doBuffer(AfBuffer &buffer) { for(AscFilter* filter : filters) filter->doBuffer(buffer); }
};

}
}
#endif
