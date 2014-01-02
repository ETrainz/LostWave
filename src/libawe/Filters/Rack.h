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

    inline void doM(Afloat &m) { for(AscFilter* filter : filters) filter->doM(m); }
    inline void doL(Afloat &l) { for(AscFilter* filter : filters) filter->doL(l); }
    inline void doR(Afloat &r) { for(AscFilter* filter : filters) filter->doR(r); }

    inline void doM(Aint   &m) { for(AscFilter* filter : filters) filter->doM(m); }
    inline void doL(Aint   &l) { for(AscFilter* filter : filters) filter->doL(l); }
    inline void doR(Aint   &r) { for(AscFilter* filter : filters) filter->doR(r); }

    inline void doF(Asfloatf &f) { for(AscFilter* filter : filters) filter->doF(f); }
    inline void doF(Asintf   &f) { for(AscFilter* filter : filters) filter->doF(f); }

    inline Afloat ffdoM(Afloat const &m) { Afloat M = m; for(AscFilter* filter : filters) filter->ffdoM(M); return M; }
    inline Afloat ffdoL(Afloat const &l) { Afloat L = l; for(AscFilter* filter : filters) filter->ffdoL(L); return L; }
    inline Afloat ffdoR(Afloat const &r) { Afloat R = r; for(AscFilter* filter : filters) filter->ffdoR(R); return R; }

    inline Afloat ifdoM(Aint const &m) { Aint M = m; for(AscFilter* filter : filters) filter->iidoM(M); return to_Afloat(M); }
    inline Afloat ifdoL(Aint const &l) { Aint L = l; for(AscFilter* filter : filters) filter->iidoL(L); return to_Afloat(L); }
    inline Afloat ifdoR(Aint const &r) { Aint R = r; for(AscFilter* filter : filters) filter->iidoR(R); return to_Afloat(R); }

    inline Aint iidoM(Aint const &m) { Aint M = m; for(AscFilter* filter : filters) filter->iidoM(M); return M; }
    inline Aint iidoL(Aint const &l) { Aint L = l; for(AscFilter* filter : filters) filter->iidoL(L); return L; }
    inline Aint iidoR(Aint const &r) { Aint R = r; for(AscFilter* filter : filters) filter->iidoR(R); return R; }

    inline Aint fidoM(Afloat const &m) { Afloat M = m; for(AscFilter* filter : filters) filter->ffdoM(M); return to_Aint(M); }
    inline Aint fidoL(Afloat const &l) { Afloat L = l; for(AscFilter* filter : filters) filter->ffdoL(L); return to_Aint(L); }
    inline Aint fidoR(Afloat const &r) { Afloat R = r; for(AscFilter* filter : filters) filter->ffdoR(R); return to_Aint(R); }

    inline Asfloatf ffdoF(Asfloatf const &f) { Asfloatf F = f; for(AscFilter* filter : filters) filter->ffdoF(F); return F; }
    inline Asintf   iidoF(Asintf   const &f) { Asintf   F = f; for(AscFilter* filter : filters) filter->iidoF(F); return F; }
};

}
}
#endif
