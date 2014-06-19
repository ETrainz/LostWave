//  UI/SwitchButton.hpp :: Switch button UI Component
//  Copyright 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Common.hpp"

namespace UI {

class SwitchButton : public clan::GUIComponent {
public:
    using ButtonState       = Common::Switch;
    using ProximityState    = Common::Switch;

protected:
    ButtonState     mButtonState;
    ProximityState  mProximityState;

public:
    clan::Colorf    mcBorder    [5]; // 2 bits :: LSB = hovered?, MSB = pushed?, last part (100) if pressing!
    clan::Colorf    mcBackground[5]; // So, 0 is unhovered and unpushed, 2 is pushed and unhovered, 3 is pushed and hovered

private:
    ////    Class callbacks    ////
    clan::Callback<void(bool)> mfButtonStateChanged;
    clan::Callback<void(bool)> mfProximityStateChanged;

public:
    SwitchButton(clan::GUIComponent *parent);

    inline bool isOn() const { return Common::isOn(mButtonState); }
    clan::Callback<void(bool)> &func_toggled() { return mfButtonStateChanged; }

    ////    Class GUI callback listeners    ////
    void on_render(clan::Canvas &canvas, recti const &clipRect);
    bool on_input (clan::InputEvent const &event);

    void on_pointer_lbtn_on (clan::InputEvent const &event);
    void on_pointer_lbtn_off(clan::InputEvent const &event);

    bool on_pointer_enter();
    bool on_pointer_leave();
};

}
#endif
