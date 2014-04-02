#include "Game.hpp"

#include "Note.hh"      // Declare global audio manager on Note.hh

Game::Game(clan::DisplayWindow &_clDW, clan::GUIManager &_clUI) :
    GUIComponent(&_clUI, { recti{ 0, 0, _clDW.get_gc().get_size() }, false }, "Game"),
    conf("config.json"),
    skin("skin.json"),
    clDW(_clDW),
    clUI(_clUI),
    clCv(clDW),
    clGC(clCv.get_gc()),
    am  (conf.getInteger("audio.frame-rate"), conf.getInteger("audio.sample-rate")),
    im  (clDW.get_ic())
{
    func_input().set(this, &Game::process_input);
    clDW.sig_window_close().connect(this, &Game::on_window_close);
    Note::am = &am;
    // TODO compile list of keys that would be used in the game and
    // notify InputManager to listen to these keys
}

void Game::on_window_close()
{
    // TODO confirm exit dialog
    clUI.exit_with_code(0);
}

bool Game::process_input(clan::InputEvent const &event)
{
    if (debug)
        dump_event(event, "Game");

    if (event.device.get_type() == clan::InputDevice::Type::keyboard)
        if (event.type == clan::InputEvent::Type::pressed)
            if (event.id   == clan::InputCode::keycode_escape) {
                this->exit_with_code(0);
                return true;
            }
    return false;
}

Game* Game::create(clan::DisplayWindowDescription &clDWD)
{
    clan::DisplayWindow clDW(clDWD);
    clan::GUIManager    clUI(clDW, "Theme");

    Game* game = new Game(clDW, clUI);

    return game;
}
