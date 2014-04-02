#include "Timer.hpp"

namespace UI {

Timer::Timer(
    clan::GUIComponent *parent,
    clan::Colorf const &bg,
    clan::Colorf const &fg
)   : clan::GUIComponent(parent, "Timer Counter")
    , mBG       (bg)
    , mFG       (fg)
    , mArray    ()
    , mNextIndex(0)
    , mLastStamp(Timer::Clock::now())
{
    set_geometry(recti{parent->get_width() - 256, 0, parent->get_width(), 256});
    func_render().set(this, &Timer::on_render);
}

void Timer::set(uchar const &skip) {
    auto   const t = std::chrono::steady_clock::now();
    ushort const s = std::chrono::duration_cast<Timer::TimeUnit>(t - mLastStamp).count() / skip;
    for(uchar i = skip; i > 0; i -= 1) {
        mArray[mNextIndex++] = s;
        mNextIndex = mNextIndex % 256;
    }
    mLastStamp = t;
    request_repaint();
}

void Timer::on_render(clan::Canvas &canvas, recti const &clipRect) {
    for(uchar i = 0; i < 255; i++)
        canvas.draw_line(i, 0, i, mArray[i], mBG);

    canvas.draw_line(mNextIndex, 0, mNextIndex, mArray[mNextIndex], mFG);
}

}
