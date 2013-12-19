//  UI/MusicSelector.hpp :: Music selection UI component
//  Copyright 2013 Keigen Shu

#ifndef UI_MUSIC_SELECTOR_H
#define UI_MUSIC_SELECTOR_H

#include "../__zzCore.hpp"
#include "../JSON.hpp"
#include "../Music.hpp"

namespace UI {

/**
 * Music Selector UI (a.k.a. Music Wheel)
 * Manages the selection of a music object.
 */
class MusicSelector : public clan::GUIComponent
{
private:
    MusicList   mMusicList;

    ////    Style elements    /////////////////////////////////////////

    point2f const mso;      // List starting offset
    vec2f const mAeo, mIeo; // Position offset per element
    vec2f const mAao, mIao; // Artist text offset from element origin
    vec2f const mAto, mIto; // Title text offset from element origin

    point2f const mSgo; // Selected element genre text offset
    point2f const mSao; // Selected element artist text offset
    point2f const mSto; // Selected element title text offset
    point2f const mSno; // Selected element notecharter text offset
    point2f const mSdo; // Selected element difficulty text offset
    point2f const mSbo; // Selected element tempo(BPM) text offset

    clan::Font mAaf, mIaf;  // List artist font
    clan::Font mAtf, mItf;  // List title font

    clan::Font mShf;    // Selected head font
    clan::Font mSbf;    // Selected body font


    ////    State variables    ////////////////////////////////////////

    // TODO Move List display UI code to a Base class.
    int mLdelay;    // Pointer movement locking period ; time taken before pointer can move to the next element.
    int mLcounter;  // Pointer movement locking counter; time elapsed since pointer last moved.

    int mVcount;    // Maximum number of elements to display on UI
    int mVtop;      // offset to top visible element
    int mVptr;      // offset to currently selected element
    int mVprv;      // Offset to previously selected element
    int mVstore;    // Storage for if user cancels

    clan::Image mBGImg; // Current background image.

public:
    // Constructor
    MusicSelector(clan::GUIComponent *owner, JSONReader &skin, MusicList const &list);

    Music* get() const;

    ////    GUI Component Callbacks    ////////////////////////////////
    bool process_input(clan::InputEvent const &event);
    void render(clan::Canvas &canvas, recti const &clip_rect);
};

}

#endif
