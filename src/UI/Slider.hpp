//  UI/Slider.hpp :: Custom slider
//  Copyright 2014 Keigen Shu

#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "../__zzCore.hpp"

namespace UI
{

class Slider : public clan::GUIComponent
{
public:
    enum Direction {
        RIGHT = 0x0,
        LEFT  = 0x1,
        DOWN  = 0x2,
        UP    = 0x3
    };

private:
    Direction mDirection;
    int mMin;   /// The lower limit of the range of the slider
    int mMax;   /// The upper limit of the range of the slider
    int mValue; /// Current value of the slider.

    clan::Callback<void()> mfValueChanged;

    ////    Slider Thumb state variables    ////

    bool        mqThumbHold;        /// Is the slider thumb being held down?
    point2i     m_PointerPosition;  /// Location of the mouse pointer on the previous update.

public:
    Slider(clan::GUIComponent *parent);

    ////    Class Attributes [Getter ans setters]    ////
    inline int get_min() const { return mMin; }
    inline int get_max() const { return mMax; }
    inline int get_position() const { return mValue; }

    inline Direction const &get_direction() const { return mDirection; }
    inline void set_direction(Direction const &d) { mDirection = d; }

    inline void set_position(int pos) { mValue = pos; }
    inline void set_ranges(int a, int b, int tick, int page) {
        mMin = std::min(a, b);
        mMax = std::max(a, b);
    }

    ////    Class interaction callback    ////
    inline clan::Callback<void()>& func_value_changed() { return mfValueChanged; }

    ////    Class GUI callback listeners    ////
    void on_render(clan::Canvas &canvas, recti const &clipRect);
    bool on_input(clan::InputEvent const &event);

    ////    Class GUI callback listener functions    ////
    void on_pointer_move(clan::InputEvent const &event);
    void on_pointer_drag(clan::InputEvent const &event);
    void on_pointer_lbtn_on(clan::InputEvent const &event);
    void on_pointer_lbtn_off(clan::InputEvent const &event);
};


}

#endif

