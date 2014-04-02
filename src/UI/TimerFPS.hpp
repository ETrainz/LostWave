//  UI/TimerFPS.hpp :: FPS counter
//  Copyright 2014 Keigen Shu

#ifndef UI_TIMER_FPS_H
#define UI_TIMER_FPS_H

#include "Timer.hpp"
namespace UI {

class TimerFPS : public Timer {
public:
    TimerFPS(
        clan::GUIComponent *parent,
        clan::Colorf const &bg = { 1.0f, 1.0f, 1.0f, 0.6f },
        clan::Colorf const &fg = { 1.0f, 0.0f, 0.0f, 0.8f }
        );

    void on_render_fps(clan::Canvas &canvas, recti const &clipRect);
};

}
#endif
