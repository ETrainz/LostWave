#include "Graph.hpp"

namespace UI {

Graph::Graph(
    clan::GUIComponent *parent
    , clan::Colorf const &plot_color
    , clan::Colorf const &curr_color
    , Graph::PlotType const &type
) : clan::GUIComponent(parent, "Grapher")
    , mcPlot    (plot_color)
    , mcThis    (curr_color)
    , mType     (type)
    , mArray    ()
    , mNextIndex(0)
{
    set_constant_repaint(false);
    set_geometry( recti {
            parent->get_width() - 256,   0,
            parent->get_width()      , 256
        } );

    func_render().set(this, &Graph::on_render);
}

void Graph::set_next(ushort const &value)
{
    mArray[mNextIndex++] = value;
    mNextIndex = mNextIndex % 256;

    request_repaint();
}

void Graph::on_render(clan::Canvas &canvas, recti const &clipRect)
{
    const uchar j = mNextIndex ? mNextIndex - 1 : 255;

    switch (mType) {
        case PlotType::POINT:
            for(uchar i = 0; i < 255; i++) {
                canvas.draw_point (i, mArray[i], mcPlot);
            }
                canvas.fill_circle(j, mArray[j], 2.0f, mcThis);
            return;
        case PlotType::LINE:
            for(uchar i = 0; i < 254; i++) {
                canvas.draw_line(i, mArray[i], i+1, mArray[i+1], mcPlot);
            }
                canvas.draw_line(j, 0, j, mArray[j], mcThis);
            return;
        case PlotType::BAR:
            for(uchar i = 0; i < 255; i++) {
                canvas.draw_line(i, 0, i, mArray[i], mcPlot);
            }
                canvas.draw_line(j, 0, j, mArray[j], mcThis);
            return;
    }

}

}