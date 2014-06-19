//  AudioTrack.hpp :: Game audio track with UI bindings
//  Copyright 2014 Keigen Shu

#ifndef AUDIO_TRACK_H
#define AUDIO_TRACK_H

#include <ClanLib/gui.h>
#include "__zzCore.hpp"
#include "libawe/Sources/Track.h"
#include "libawe/Filters/3BEQ.h"
#include "libawe/Filters/Mixer.h"
#include "libawe/Filters/Metering.h"
#include "UI/Slider.hpp"
#include "UI/SwitchButton.hpp"

/* Margin 1px; Padding 2px;
 */
class AudioTrack : public clan::GUIComponent
{
private:
    awe::Source::Atrack         *mTrack;

    awe::Filter::TBEQ<2>        *m3BEQ;
    awe::Filter::AscMixer       *mMixer;
    awe::Filter::AscMetering    *mMeter;

    ////    GUI Controls    ///////////////////////////////////////////
    UI::Slider                  mGCsdvEQGainL;
    UI::Slider                  mGCsdvEQGainM;
    UI::Slider                  mGCsdvEQGainH;

    UI::Slider                  mGCsdhEQFreqL;
    UI::Slider                  mGCsdhEQFreqH;

    UI::Slider                  mGCsdvGain;
    UI::Slider                  mGCsdhPan;

    UI::SwitchButton            mGCbtnMute;
    UI::SwitchButton            mGCbtnToggleSize;

public:
    AudioTrack(
        awe::Source::Atrack *source,
        clan::GUIComponent  *parent,
        point2i              area
    );

    ~AudioTrack();

    static sizei _getSize(bool mini = false);

    awe::Source::Atrack       *  getTrack()       { return mTrack; }
    awe::Source::Atrack const * cgetTrack() const { return mTrack; }

    ////    GUI Component Methods    //////////////////////////////////
    void render(clan::Canvas &canvas, const recti &clip_rect);

    void eq_gain_changed();
    void eq_freq_changed();

    void mixer_value_changed();

    void mute_toggled(bool);
    void size_toggled(bool);
};


#endif
