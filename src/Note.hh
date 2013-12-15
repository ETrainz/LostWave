//  Note.hh :: Abstract Note class
//  Copyright 2013 Keigen Shu

#ifndef NOTE_HH
#define NOTE_HH

#include <cstdint>
#include <list>
#include <map>
#include <set>
#include "Chrono.hpp"
#include "Judge.hpp"

namespace clan { class Canvas;  }
namespace UI   { class Tracker; }
class AudioManager;

enum class KeyStatus : uint8_t;

// Note Key enumerator
enum class ENKey : uint8_t
{
    NOTE_P1_S = 0x10, NOTE_P1_1 = 0x11, NOTE_P1_2 = 0x12, NOTE_P1_3 = 0x13,
    NOTE_P1_4 = 0x14, NOTE_P1_5 = 0x15, NOTE_P1_6 = 0x16, NOTE_P1_7 = 0x17,
    NOTE_P1_8 = 0x18, NOTE_P1_9 = 0x19,
    NOTE_P2_S = 0x20, NOTE_P2_1 = 0x21, NOTE_P2_2 = 0x22, NOTE_P2_3 = 0x23,
    NOTE_P2_4 = 0x24, NOTE_P2_5 = 0x25, NOTE_P2_6 = 0x26, NOTE_P2_7 = 0x27,
    NOTE_P2_8 = 0x28, NOTE_P2_9 = 0x29,
    NOTE_AUTO = 0x30
};

typedef std::list<ENKey> KeyList;
typedef std::set <ENKey> KeySet;

/** Base class for all note classes */
class Note
{
protected:
    ENKey   key;
    TTime   time;   // real position in tick-base time
    long    tick;   // post-initialisation calculated position
    bool    dead;
    JScore  score;

public:
    static AudioManager* am;

    Note (ENKey Key, TTime Time) : key(Key), time(Time), tick(0), dead(false), score(JScore_NONE) { }
    virtual ~Note() { }

    inline const ENKey  & getKey  () const { return key; }
    inline const TTime  & getTime () const { return time; }
    inline const long   & getTick () const { return tick; }
    inline const JScore & getScore() const { return score; }
    inline       bool     isScored() const { return score.rank != EJRank::NONE; }
    inline const bool   & isDead  () const { return dead; }

    virtual void init  (UI::Tracker const &t) = 0;
    virtual void render(UI::Tracker const &t, clan::Canvas &canvas) const = 0;

    /**
     * Note scoring update function.
     * @return true to make main code remove this note from judgement context.
     */
    virtual void update(UI::Tracker const &t, KeyStatus const &key) = 0;

};

typedef std::list<Note*> NoteList;

/** Note comparator function (for use with NoteList::sort())
 * This function is used to sort notes by time and key in ascending order.
 * \code{.cpp}
 * if (a.time == b.time)
 *     return a.key  < b.key ;
 * else
 *     return a.time < b.time;
 * \endcode
 */
inline bool cmpNote_Greater(const Note * const &a, const Note * const &b)
{
    return (a->getTime() == b->getTime()) ?
        (a->getKey () < b->getKey()) :
        (a->getTime() < b->getTime());
}

template<class NoteType>
    std::map<ENKey, std::list<NoteType*>>
split_notes_by_key(
        const std::list< NoteType* >& list
        )
{
    std::map< ENKey, std::list<NoteType*> > map;
    for(NoteType* note : list)
        map[note->getKey()].push_back(note);

    for(auto node : map)
        node.second.sort(
                [ ] ( const NoteType* const &a, const NoteType* const &b ) -> bool
                { return (a->getTime() < b->getTime()); }
                );

    return map;
}

#endif
