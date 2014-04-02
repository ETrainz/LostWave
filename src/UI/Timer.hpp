//  UI/Timer.hpp :: Response performance timer.
//  Copyright 2014 Keigen Shu

#ifndef UI_TIMER_H
#define UI_TIMER_H

#include "../__zzCore.hpp"
#include <chrono>

namespace UI {

class Timer : public clan::GUIComponent {
public:
    using Clock     = std::chrono::steady_clock;
    using TimeStamp = std::chrono::time_point<Clock>;
    using TimeUnit  = std::chrono::milliseconds;

protected:
    clan::Colorf            mBG, mFG;
    std::array<ushort, 256> mArray;
    uchar                   mNextIndex;
    TimeStamp               mLastStamp;

public:
    Timer(
        clan::GUIComponent *parent,
        clan::Colorf const &bg = { 1.0f, 1.0f, 1.0f, 0.6f },
        clan::Colorf const &fg = { 1.0f, 0.0f, 0.0f, 0.8f }
        );

    void set(uchar const &skip = 1);

    void on_render(clan::Canvas &canvas, recti const &clipRect);
};

}
#endif
