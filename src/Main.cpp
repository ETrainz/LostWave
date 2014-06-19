#include "__zzCore.hpp"

#ifdef __USE_D3D
#include <ClanLib/d3d.h>
#else
#include <ClanLib/gl.h>
#endif

#include "Main.hpp"
#include "Game.hpp"

#include "UI/MusicSelector.hpp"
#include "UI/FFT.hpp"
#include "UI/Tracker.hpp"

#include "UI/Graph.hpp"
#include "UI/Graph_Time.hpp"
#include "UI/Graph_FrameRate.hpp"

#include "AudioTrack.hpp"
#include "MusicScanner.hpp"
#include "Chart_O2Jam.hpp"
#include "Chart_BMS.hpp"

Game* App::game = nullptr;

bool noSound = true;


UI::Tracker::ChannelList default_ChannelList
{
    { ENKey::NOTE_P1_1, clan::InputCode::keycode_s },
    { ENKey::NOTE_P1_2, clan::InputCode::keycode_d },
    { ENKey::NOTE_P1_3, clan::InputCode::keycode_f },
    { ENKey::NOTE_P1_4, clan::InputCode::keycode_space },
    { ENKey::NOTE_P1_5, clan::InputCode::keycode_j },
    { ENKey::NOTE_P1_6, clan::InputCode::keycode_k },
    { ENKey::NOTE_P1_7, clan::InputCode::keycode_l }
};


static void autoVisualize(
    AudioManager* am
    , UI::FFT* FFTbg
    , UI::FFT* FFTp1
    , UI::FFT* FFTp2
    , UI::Graph_Time*   graph
    , std::vector<UI::Graph*>   graph_m
) {
#if !( defined(_WIN32) || defined(_WIN64) )
    pthread_setname_np(pthread_self(), "Audio VFX");
#endif

    ulong count = 0;
    awe::Filter::Maximizer<2> *pMaxer =
            reinterpret_cast< awe::Filter::Maximizer< 2 >* >(
                am->getMasterTrack().getRack().getFilter(3)
            );

    while(am->getRunning().test_and_set())
    {
        if (am->getMutex().try_lock())
        {
            ulong new_count = am->getUpdateCount();
            if (new_count != count)
            {
                FFTbg->update_2(am->getMasterTrack().getOutput());
                FFTp1->update_2(am->getTrackMap()->at(1)->getOutput());
                FFTp2->update_2(am->getTrackMap()->at(2)->getOutput());
                graph->set_time(new_count - count);

                graph_m[0]->set_next(-awe::dBFS_limit / pMaxer->getCurrentGain());
                graph_m[1]->set_next(-awe::dBFS_limit * pMaxer->getThreshold());
                graph_m[2]->set_next(-awe::dBFS_limit * pMaxer->getPeakSample());
                count = new_count;
            }
            am->getMutex().unlock();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    am->getRunning().clear();
}

int App::main(std::vector<std::string> const& args)
{
    clan::SetupCore     _SetupCore;
    clan::SetupDisplay  _SetupDisplay;
    clan::SetupGUI      _SetupGUI;

#ifdef __USE_D3D
    clan::SetupD3D      _SetupD3DRender;
#else
    clan::SetupGL       _SetupGLRender;
#endif

    try {
        JSONFile config("config.json");
        debug = config.get_or_set(
                &JSONReader::getBoolean, "debug", false
                );

        sizei resolution = config.get_if_else_set(
                &JSONReader::getVec2i, "video.resolution", vec2i(640, 480),
                [] (vec2i const &value) -> bool {
                    return value.x > 640 && value.y > 480;
                });

        clan::DisplayWindowDescription clDWD(
                "LostWave", resolution, true
                );

        clDWD.set_allow_resize      (false);
        clDWD.set_allow_screensaver (false);
        clDWD.set_fullscreen        (config.getBoolean("video.fullscreen"));
        clDWD.set_refresh_rate      (config.getInteger("video.refresh-rate"));
        clDWD.set_swap_interval     (config.getInteger("video.flip-interval"));

        game = Game::create(clDWD);

        UI::Graph_FrameRate* graphFR = new UI::Graph_FrameRate(game);

        {   // Create audio visuals thread
            uint FFTbars = config.get_if_else_set(
                    &JSONReader::getInteger, "audio.fft.bars", 128,
                    [](long const & value) -> bool {
                        return value > 0 && value < 65536;
                    });

            float FFTfade = config.get_if_else_set(
                    &JSONReader::getDecimal, "audio.fft.fade", 1.0,
                    [](double const & value) -> bool {
                        return value > 0.0;
                    });

            std::string fftw = config.get_if_else_set(
                    &JSONReader::getString, "audio.fft.window", std::string{"Hanning"},
                    [] (std::string const & value) -> bool {
                        std::string str;
                        std::transform(
                            value.begin(), value.end(), str.begin(), ::tolower
                            );

                        if (str.compare(    "hanning") == 0) return true;
                        if (str.compare(    "lanczos") == 0) return true;
                        if (str.compare("rectangular") == 0) return true;
                        if (str.compare( "triangular") == 0) return true;

                        return false;
                    });

            UI::FFT::IEWindowType FFTwindow
                = (fftw.compare(    "lanczos") == 0) ? UI::FFT::IEWindowType::LANCZOS
                : (fftw.compare("rectangular") == 0) ? UI::FFT::IEWindowType::RECTANGULAR
                : (fftw.compare( "triangular") == 0) ? UI::FFT::IEWindowType::TRIANGULAR
                                                     : UI::FFT::IEWindowType::HANNING
                ;

            UI::FFT* FFTbg = new UI::FFT(
                    game, game->get_geometry(),
                    game->am.getMasterTrack().getConfig().targetFrameCount,
                    game->am.getMasterTrack().getConfig().targetSampleRate,
                    FFTbars, FFTfade, FFTwindow
                    );
            UI::FFT* FFTp1 = new UI::FFT(
                    game, game->get_geometry(),
                    game->am.getMasterTrack().getConfig().targetFrameCount,
                    game->am.getMasterTrack().getConfig().targetSampleRate,
                    FFTbars, FFTfade, FFTwindow
                    );
            UI::FFT* FFTp2 = new UI::FFT(
                    game, game->get_geometry(),
                    game->am.getMasterTrack().getConfig().targetFrameCount,
                    game->am.getMasterTrack().getConfig().targetSampleRate,
                    FFTbars, FFTfade, FFTwindow
                    );

            std::vector<UI::Graph*> graphMG;

            graphMG.push_back(new UI::Graph(game,
                    clan::Colorf {1.0f, 1.0f, 1.0f, 0.6f},
                    clan::Colorf {1.0f, 1.0f, 1.0f, 0.8f},
                    UI::Graph::PlotType::LINE
                    ));
            graphMG.push_back(new UI::Graph(game,
                    clan::Colorf {1.0f, 0.0f, 1.0f, 0.4f},
                    clan::Colorf {1.0f, 0.0f, 1.0f, 0.6f},
                    UI::Graph::PlotType::POINT
                    ));
            graphMG.push_back(new UI::Graph(game,
                    clan::Colorf {0.0f, 1.0f, 1.0f, 0.2f},
                    clan::Colorf {0.0f, 1.0f, 1.0f, 0.4f},
                    UI::Graph::PlotType::BAR
                    ));

            UI::Graph_Time* graphAE = new UI::Graph_Time(
                    game,
                    clan::Colorf { 1.0f, 0.8f, 0.0f, 0.6f },
                    clan::Colorf { 1.0f, 1.0f, 0.0f, 0.8f }
                    );

            AudioTrack* ATPM = new AudioTrack(&game->am.getMasterTrack()   , game, point2i{ 600, game->get_height() - AudioTrack::_getSize().height });
            AudioTrack* ATP0 = new AudioTrack(game->am.getTrackMap()->at(0), game, point2i{ 650, game->get_height() - AudioTrack::_getSize().height });
            AudioTrack* ATP1 = new AudioTrack(game->am.getTrackMap()->at(1), game, point2i{ 700, game->get_height() - AudioTrack::_getSize().height });
            AudioTrack* ATP2 = new AudioTrack(game->am.getTrackMap()->at(2), game, point2i{ 750, game->get_height() - AudioTrack::_getSize().height });

            {   // Set-up Master Output Maximizer
                float MOMBoost = config.get_if_else_set(
                        &JSONReader::getDecimal, "audio.maximizer.boost"       , 0.0,
                        [](double const & value) -> bool {
                            return value > 0.0;
                        });
                float MOMThreshold = config.get_if_else_set(
                        &JSONReader::getDecimal, "audio.maximizer.threshold"   , -3.0,
                        [](double const & value) -> bool {
                            return value < 0.0;
                        });
                int MOMPeakRelease = config.get_if_else_set(
                        &JSONReader::getInteger, "audio.maximizer.peak-release", 4,
                        [](int const & value) -> bool {
                            return value > 0;
                        });
                int MOMSlowRelease = config.get_if_else_set(
                        &JSONReader::getInteger, "audio.maximizer.slow-release", 400,
                        [](int const & value) -> bool {
                            return value > 0;
                        });
                float MOMCeiling = config.get_if_else_set(
                        &JSONReader::getDecimal, "audio.maximizer.ceiling"     , 3.0,
                        [](double const & value) -> bool {
                            return value > awe::dBFS_limit && value < 3.0;
                        });

                ATPM->getTrack()->getRack().attach_filter(new awe::Filter::Maximizer<2>(
                        ATPM->getTrack()->getConfig().targetSampleRate / 2,
                        awe::from_dBFS(MOMBoost),
                        awe::from_dBFS(MOMThreshold),
                        MOMSlowRelease,
                        MOMPeakRelease,
                        awe::from_dBFS(MOMCeiling)
                        ));
            }

            std::thread* av_thread = new std::thread(
                    autoVisualize, &game->am, FFTbg, FFTp1, FFTp2, graphAE, graphMG
                    );
            game->am.attach_thread(av_thread);
        }

        if (noSound == true) {
            awe::ArenderConfig arc = game->am.getMasterTrack().getConfig();
            arc.quality = awe::ArenderConfig::Quality::SKIP;
            game->am.getMasterTrack().setConfig(arc);
        }

        if (args.size() > 1)
        {
            // TODO read other parameters
            for(size_t a = 1; a < args.size(); a++)
            {
                if (clan::PathHelp::get_extension(args[a]).compare("ojn") == 0)
                {
                    Music* music = O2Jam::openOJN(args[a]);
                    launchChart(music->charts[0]);
                    delete music;
                } else {
                    Chart_BMS* chart = new Chart_BMS(args[a]);
                    launchChart(chart);
                    delete chart;
                }
            }
        } else {
            MusicList           ML = scan_music_dir("./Music");
            UI::MusicSelector   MS { game, game->skin, ML };

            while(MS.exec() == 0)
            {
                MS.set_enabled(false);
                MS.set_visible(false);

                launchChart(MS.get());

                MS.set_enabled(true);
                MS.set_visible(true);
            }
        }

    } catch (clan::Exception& exception) {
        clan::ConsoleWindow console("Console", 80, 160);
        clan::Console::write_line("Exception caught: " + exception.get_message_and_stack_trace());
        console.display_close_message();
        return 1;
    }

    return 0;
}

void App::launchChart(Chart* chart)
{
    if (chart == nullptr)
        return;

    chart->load_art();
    chart->load_samples();

    game->am.wipe_SampleMap(true);
    game->am.swap_SampleMap(chart->getSampleMap());

    chart->load_chart();
    chart->sort_sequence();

    recti chart_area { 50, 50, 450, game->get_height() - 50 };

    UI::Tracker::ChannelList channels(default_ChannelList);

    {   // Populate input keys based on config.
        std::string input_str = game->conf.get_if_else_set(
                &JSONReader::getString, "player.P1.map-7K", std::string("sdf jkl"),
                [](std::string const & value) -> bool { return value.size() == 7; }
                );
#ifdef _MSC_VER
        std::transform(input_str.begin(), input_str.end(),
                       input_str.begin(), ::toupper);
#else
        std::transform(input_str.begin(), input_str.end(),
                       input_str.begin(), ::tolower);
#endif
        auto chIter = channels.begin();

        for (char const & c : input_str) {
            chIter->code = c;
            chIter++;
        }
    }

    UI::Tracker tracker(game, chart_area, JHard, chart, channels);
    tracker.start();
    tracker.exec ();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
