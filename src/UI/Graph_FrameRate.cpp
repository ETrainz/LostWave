#include "Graph_FrameRate.hpp"

namespace UI {

Graph_FrameRate::Graph_FrameRate(clan::GUIComponent *parent) : Graph_Time(parent)
{
    func_render().set(this, &Graph_FrameRate::on_render_fr);
    set_constant_repaint(true);
}

void Graph_FrameRate::on_render_fr(clan::Canvas& canvas, const recti& clipRect)
{
    set_time(1);
    on_render(canvas, clipRect);
}

}
