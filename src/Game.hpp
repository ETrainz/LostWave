#ifndef GAME_H
#define GAME_H

#include <random>
#include <ClanLib/display.h>
#include <ClanLib/gui.h>
#include "JSON.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"

/** Main game object */
class Game : public clan::GUIComponent
{
public:
    JSONFile conf;
    JSONFile skin;

    clan::DisplayWindow     clDW;
    clan::GUIManager        clUI;
    clan::Canvas            clCv;
    clan::GraphicContext    clGC;

    AudioManager            am;
    InputManager            im;

private:
    Game(clan::DisplayWindow &_clDW, clan::GUIManager &_clUI);

    // Signals
    void on_window_close();

    bool process_input(clan::InputEvent const &event);

public:
    static Game* create(clan::DisplayWindowDescription &_clDWD);
};

#endif
