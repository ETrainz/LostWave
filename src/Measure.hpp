//  Measure.hpp :: Measures and Music sequences
//  Copyright 2011 - 2013 Keigen Shu

#ifndef MEASURE_H
#define MEASURE_H

#include <list>
#include "ParamEvent.hpp"
#include "Note.hpp"

class Measure
{
private:
    NoteList        lNotes;
    ParamEventList  lParams;

    unsigned beatCount;   // a - beats per measure
    unsigned beatSize;    // b - ticks per beat
    unsigned tickCount;   // total number of ticks in this measure

public:
    Measure (unsigned a, unsigned b): beatCount(a), beatSize(b), tickCount(a*b) {}
    Measure (double z) { setTimeSignature(z); }
    ~Measure() { lNotes.clear(); lParams.clear(); }

    inline void addNote (Note* _note) { lNotes.push_back(_note); }
    inline void addParamEvent (ParamEvent* _param) { lParams.push_back(_param); }

    inline NoteList             &  getNotes () { return lNotes ; }
    inline ParamEventList       &  getParams() { return lParams; }
    inline NoteList       const & cgetNotes () const { return lNotes ; }
    inline ParamEventList const & cgetParams() const { return lParams; }

    void setTimeSignature(unsigned a, unsigned b);
    void setTimeSignature(double z);

    TTime getTimeFromTickCount(unsigned t) const;

    inline unsigned getA() const { return beatCount; }
    inline unsigned getB() const { return beatSize; }
    inline unsigned getTickCount() const { return tickCount; }

    /** Sorts listed objects in chronological order. */
    void sort_lists ();
};

typedef std::vector<Measure*> Sequence;

#endif
