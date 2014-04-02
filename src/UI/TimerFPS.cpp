#include "TimerFPS.hpp"

namespace UI {

TimerFPS::TimerFPS(
        clan::GUIComponent *parent,
        clan::Colorf const &bg,
        clan::Colorf const &fg
)   : Timer(parent, bg, fg)
{
    func_render().set(this, &TimerFPS::on_render_fps);
    set_constant_repaint(true);
}

void TimerFPS::on_render_fps(clan::Canvas& canvas, const recti& clipRect)
{
    set();
    on_render(canvas, clipRect);
}

}
