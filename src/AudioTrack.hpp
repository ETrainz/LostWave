//  AudioTrack.hpp :: Game audio track with UI bindings
//  Copyright 2014 Keigen Shu

#ifndef AUDIO_TRACK_H
#define AUDIO_TRACK_H

#include <ClanLib/gui.h>
#include "__zzCore.hpp"
#include "libawe/aweTrack.h"
#include "libawe/Filters/3BEQ.h"
#include "libawe/Filters/Mixer.h"
#include "libawe/Filters/Metering.h"

/* Margin 1px; Padding 2px;
 */
class AudioTrack : public clan::GUIComponent
{
private:
    awe::Atrack                 *mTrack;

    awe::Filter::Asc3BEQ        *m3BEQ;
    awe::Filter::AscMixer       *mMixer;
    awe::Filter::AscMetering    *mMeter;

    ////    GUI Controls    ///////////////////////////////////////////
    clan::Slider                mGCsdvEQGainL;
    clan::Slider                mGCsdvEQGainM;
    clan::Slider                mGCsdvEQGainH;

    clan::Slider                mGCsdhEQFreqL;
    clan::Slider                mGCsdhEQFreqH;

    clan::Slider                mGCsdvGain;
    clan::Slider                mGCsdhPan;
    /*
    clan::Button                mGCbtnMute;
    clan::Button                mGCbtnToggleSize;
    */

public:
    AudioTrack(
        awe::Atrack        *source,
        clan::GUIComponent *parent,
        point2i             area
    );

    ~AudioTrack();

    ////    GUI Component Methods    //////////////////////////////////
    void render(clan::Canvas &canvas, const recti &clip_rect);

    void eq_gain_changed();
    void eq_freq_changed();

    void mixer_value_changed();
};


#endif
