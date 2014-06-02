#include "SwitchButton.hpp"
#include <cassert>

namespace UI {

SwitchButton::SwitchButton(clan::GUIComponent *parent)
    : clan::GUIComponent(parent, "SwitchButton")
    , mButtonState      (Common::Switch::OFF)
    , mProximityState   (Common::Switch::OFF)

{
    ////    Default colors    ////
    mcBorder[0] = {0.4f, 0.4f, 0.4f}; // not pressed
    mcBorder[1] = {0.3f, 0.3f, 0.3f}; // PRESSED
    mcBorder[2] = {0.6f, 0.6f, 0.8f}; // HOVER + not pressed
    mcBorder[3] = {0.5f, 0.5f, 0.7f}; // HOVER + PRESSED
    mcBorder[4] = {0.2f, 0.2f, 0.2f}; // PRESSING

    mcBackground[0] = {0.8f, 0.8f, 0.8f};
    mcBackground[1] = {0.6f, 1.0f, 0.6f};
    mcBackground[2] = {0.8f, 0.8f, 1.0f};
    mcBackground[3] = {0.8f, 1.0f, 1.0f};
    mcBackground[4] = {0.4f, 1.0f, 0.6f};

    func_render().set(this, &SwitchButton::on_render);
    func_input ().set(this, &SwitchButton::on_input );
    func_pointer_enter().set(this, &SwitchButton::on_pointer_enter);
    func_pointer_exit ().set(this, &SwitchButton::on_pointer_leave);

    set_constant_repaint(false);
}

void SwitchButton::on_render(clan::Canvas &canvas, recti const &clipRect)
{
    uchar i = Common::isChanging(mButtonState) ?
        4 : ( 0 +
        ((mProximityState & Common::Switch::__STATE) << 1) +
        ((mButtonState    & Common::Switch::__STATE) << 0) );

    canvas.fill_rect(            1,              1, get_width()-1, get_height()  , mcBackground[i]);

    canvas.draw_line(            1,              1, get_width()-1,              1, mcBorder[i]); // Top
    canvas.draw_line(            1,              1,             1, get_height()-1, mcBorder[i]); // Left
    canvas.draw_line(get_width()-1,              1, get_width()  , get_height()-1, mcBorder[i]); // Right
    canvas.draw_line(            1, get_height()-1, get_width()-1, get_height()  , mcBorder[i]); // Bottom
}

bool SwitchButton::on_input(clan::InputEvent const &event)
{
    if (debug)
        dump_event(event, "SwitchButton");

    if (event.device.get_type() == clan::InputDevice::Type::pointer)
    {
        switch (event.type)
        {
            case clan::InputEvent::Type::pressed:
                if (event.id == clan::InputCode::mouse_left) {
                    on_pointer_lbtn_on(event);
                    return true;
                }
                break;

            case clan::InputEvent::Type::released:
                if (event.id == clan::InputCode::mouse_left) {
                    on_pointer_lbtn_off(event);
                    return true;
                }
                break;

            case clan::InputEvent::Type::axis_moved:
                // TODO: Implement Scroll wheel
                break;
        }
    }

    return true;
}

void SwitchButton::on_pointer_lbtn_on(clan::InputEvent const &event) {
    assert(event.device.get_type() == clan::InputDevice::Type::pointer);
    assert(event.id == clan::InputCode::mouse_left);
    assert(event.type == clan::InputEvent::Type::pressed);

    Common::setChanging(mButtonState);
    request_repaint();
}

void SwitchButton::on_pointer_lbtn_off(clan::InputEvent const &event) {
    assert(event.device.get_type() == clan::InputDevice::Type::pointer);
    assert(event.id == clan::InputCode::mouse_left);
    assert(event.type == clan::InputEvent::Type::released);

    Common::resetChanging(mButtonState);
    mButtonState = ~mButtonState;

    if (mfButtonStateChanged.is_null() == false)
        mfButtonStateChanged.invoke(Common::isOn(mButtonState));

    request_repaint();
}

bool SwitchButton::on_pointer_enter() {
    mProximityState = +mProximityState;
    request_repaint();
    return true;
}

bool SwitchButton::on_pointer_leave() {
    mProximityState = -mProximityState;
    request_repaint();
    return true;
}

}
