//  Main.cpp :: Application main code
//  Copyright 2013 Keigen Shu

#include "__zzCore.hpp"
#include "Main.hpp"
#include "JSON.hpp"
#include "Game.hpp"

#include "UI/MusicSelector.hpp"
#include "UI/FFT.hpp"
#include "UI/Tracker.hpp"
#include "MusicScanner.hpp"
#include "Chart_O2Jam.hpp"
#include "Chart_BMS.hpp"

#define ENABLE_VISUALIZER true

// TODO constexpr pairs are not supported until C++14
std::initializer_list< std::pair<const ENKey, InputManager::KeyCode> >
default_keymap {
    { ENKey::NOTE_P1_1, clan::InputCode::keycode_s },
    { ENKey::NOTE_P1_2, clan::InputCode::keycode_d },
    { ENKey::NOTE_P1_3, clan::InputCode::keycode_f },
    { ENKey::NOTE_P1_4, clan::InputCode::keycode_space },
    { ENKey::NOTE_P1_5, clan::InputCode::keycode_j },
    { ENKey::NOTE_P1_6, clan::InputCode::keycode_k },
    { ENKey::NOTE_P1_7, clan::InputCode::keycode_l }
};

constexpr std::initializer_list< ENKey > _0ENKey {};
constexpr std::initializer_list< ENKey >
default_keys {
    ENKey::NOTE_P1_1,
    ENKey::NOTE_P1_2,
    ENKey::NOTE_P1_3,
    ENKey::NOTE_P1_4,
    ENKey::NOTE_P1_5,
    ENKey::NOTE_P1_6,
    ENKey::NOTE_P1_7
};




static void autoVisualize(AudioManager* am, UI::FFT* FFTbg, UI::FFT* FFTfg)
{
    ulong count = 0;

    while(ENABLE_VISUALIZER)
    {
        if(am->getMutex().try_lock())
        {
            ulong new_count = am->getUpdateCount();
            if (new_count != count)
            {
                FFTbg->update(am->getMasterTrack().getOutput());
                FFTfg->update(am->getTrackMap()->at(1)->getOutput());
                count = new_count;
            }
            am->getMutex().unlock();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

int App::main(std::vector<std::string> const &args)
{
    clan::SetupCore     _SetupCore;
    clan::SetupDisplay  _SetupDisplay;
    clan::SetupGL       _SetupGLRender;
    clan::SetupGUI      _SetupGUI;

    try {
        JSONFile config("config.json");

        sizei resolution = config.get_if_else_set(
            &JSONReader::getVec2i, "video.resolution", vec2i(640, 480),
            [] (vec2i const &value) -> bool { return value.x > 640 && value.y > 480; }
            );

        clan::DisplayWindowDescription clDWD(
                "LostWave", resolution, true
                );

        clDWD.set_allow_resize      (false);
        clDWD.set_allow_screensaver (false);
        clDWD.set_fullscreen        (config.getBoolean("video.fullscreen"));
        clDWD.set_refresh_rate      (config.getInteger("video.refresh-rate"));
        clDWD.set_swap_interval     (config.getInteger("video.flip-interval"));

        Game* game = Game::create(clDWD);

        uint FFTbars = config.get_if_else_set(
            &JSONReader::getInteger, "audio.fft.bars", 128,
            [] (long const &value) -> bool { return value > 0 && value < 65536; }
            );

        bool autoplay = config.get_or_set(
                &JSONReader::getBoolean, "player.P1.autoplay", false);


        UI::FFT FFTbg(game, game->get_geometry(),
                    game->am.getMasterTrack().getConfig().targetFrameCount,
                    game->am.getMasterTrack().getConfig().targetSampleRate,
                    FFTbars, 0.1f
        );
        UI::FFT FFTfg(game, game->get_geometry(),
                    game->am.getMasterTrack().getConfig().targetFrameCount,
                    game->am.getMasterTrack().getConfig().targetSampleRate,
                    FFTbars, 0.1f
        );
        FFTbg.set_constant_repaint(true);
        FFTfg.set_constant_repaint(true);

        std::thread AV(autoVisualize, &game->am, &FFTbg, &FFTfg);
        AV.detach();

        Music* music;
        Chart* chart;

        if (args.size() > 1)
        {
            // TODO read other parameters
            for(size_t a=1; a<args.size(); a++)
            {
                if (clan::PathHelp::get_extension(args[a]).compare("ojn") == 0) {
                    music = O2Jam::openOJN(args[a]);
                    chart = music->charts[0];
                } else {
                    Chart_BMS* bms = new Chart_BMS(args[a]);
                    chart = bms;
                }

                chart->load_art();
                chart->load_samples();
                game->am.swap_SampleMap(chart->getSampleMap());
                chart->load_chart();
                chart->sort_sequence();

                {
                    UI::Tracker tracker(
                        game, game->get_geometry(), JHard, chart,
                        default_keymap, autoplay ? default_keys : _0ENKey
                    );
                    tracker.start();
                    tracker.exec();
                }
            }
        } else {
            MusicList ML = scan_music_dir("./Music");
            UI::MusicSelector MS(game, game->skin, ML);
            while(MS.exec() == 0)
            {
                MS.set_enabled(false);
                MS.set_visible(false);

                music = MS.get();
                chart = music->charts[0];

                chart->load_art();
                chart->load_samples();
                game->am.swap_SampleMap(chart->getSampleMap());
                chart->load_chart();
                chart->sort_sequence();

                {
                    UI::Tracker tracker(
                        game, game->get_geometry(), JHard, chart,
                        default_keymap, autoplay ? default_keys : _0ENKey
                    );
                    tracker.start();
                    tracker.exec();
                }

                MS.set_enabled(true);
                MS.set_visible(true);
            }
        }

    } catch (clan::Exception &exception) {
        clan::ConsoleWindow console("Console", 80, 160);
        clan::Console::write_line("Exception caught: " + exception.get_message_and_stack_trace());
        console.display_close_message();
        return 1;
    }

    return 0;
}
