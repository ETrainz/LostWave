//  Chart.hpp :: Chart class object declaration
//  Copyright 2011 - 2013 Keigen Shu

#include "Chart.hpp"
#include "Measure.hpp"

long Chart::compare_ticks(const TTime &a, const TTime &b) const
{
    // Swap places if z comes earlier than a.
    if (a >  b) return -(compare_ticks(b, a));
    else if (a == b) return 0;

    int r = 0;

    // Count from starting point to the end
    for (unsigned int i = a.measure; i <= b.measure; i++)
    {
        const Measure* m = this->cgetMeasure(i);

        if (m == nullptr) {
            continue;
        } else if (a.measure == b.measure) {
            r += (b.beat * m->getB() + b.tick) - (a.beat * m->getB() + a.tick);
        } else if (i == a.measure) {
            r += (m->getA() -(a.beat+1)) * m->getB();
            r +=  m->getB() - a.tick;
        } else if (i == b.measure) {
            r += (b.beat * m->getB());
            r +=  b.tick;
        } else {
            r += m->getTickCount();
        }
    }

    return r;
}

double Chart::translate(const TTime &t) const
{
    TTime time  = _0TTime;
    double ret = 0.0;

    double BPM = tempo;
    ulong Tcount = 0;
    TTime prevTC = _0TTime;

    // Gather all relevant parameter events from beginning to end.
    for(unsigned int i = 0; i <= t.measure; i++)
    {
        Measure const *m = this->cgetMeasure(i);

        Tcount += compare_ticks(prevTC, TTime(0,0,i));
        prevTC  = TTime(0,0,i);

        if (m == nullptr)
            continue;

        for(ParamEvent* p : m->cgetParams())
        {
            if (p->time < t)
            {
                /****/ if (p->param == EParam::EP_C_TEMPO) {
                    Tcount += compare_ticks(prevTC, p->time);
                    prevTC  = p->time;

                    ret += static_cast<double>(Tcount) * (1.25 / BPM);
                    BPM  = p->value.asFloat;

                    Tcount  = 0;
                } else if (p->param == EParam::EP_C_STOP_T) {
                    Tcount += p->value.asInt;
                } else if (p->param == EParam::EP_C_STOP_R) {
                    ret += p->value.asFloat;
                }
            }
        }
    }

    Tcount += compare_ticks(prevTC, t);
    return ret + static_cast<double>(Tcount) * (1.25 / BPM);
}

// clears all lists and maps in the chart.
void Chart::clear()
{
    while (!sequence.empty())
    {
        Measure* m = *(sequence.begin());
        sequence.erase(sequence.begin());
        delete m;
    }
    sequence.clear();
}
