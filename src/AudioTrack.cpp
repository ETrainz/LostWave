#include "AudioTrack.hpp"

////    GUI consts    /////////////////////////////////////////////////
constexpr int   kPadding    = 4     , kmPadding     = 2;
constexpr int   kWidth      = 40    , kmWidth       = 12;

constexpr int   kSliderWidth    = 2 * kPadding + 1;


const sizei kEQGainSize = {
    static_cast<int>(static_cast<float>(kWidth - 3 * kPadding) / 3.0f),
    kWidth + 1
};

const sizei kEQFreqSize = {
    static_cast<int>(static_cast<float>(kWidth - 2 * kPadding) / 2.0f),
    kSliderWidth
};

const sizei kPanSize        = { kWidth - 2 * kPadding, kSliderWidth };
const sizei kMeterSize      = { kPadding - 1 , 120 };   // Single meter

constexpr int   kMarkerWidth    = kPadding;
const sizei kFaderSize      = {(kWidth - 3 * kPadding) / 3, 120 + 2 * kPadding };

const sizei kButtonSize     = {(kWidth - 3 * kPadding) / 2, kPadding * 2 };

const sizei kSize           = {
    kWidth,
    kEQGainSize.height + kEQFreqSize.height + kPanSize.height + kMeterSize.height + kButtonSize.height * 2 + 9 * kPadding
};

const point2i   koEQGain    = { kPadding, kPadding };
const point2i   koEQFreq    = { kPadding, koEQGain.y + kEQGainSize.height + kPadding };
const int       kySplit     = koEQFreq.y + kEQFreqSize.height + kPadding * 2;
const point2i   koPan       = { kPadding, kPadding + kySplit };
const point2i   koMeter     = { kPadding, koPan.y + kPanSize.height + kPadding };
const point2i   koButton    = { kPadding, koMeter.y + kFaderSize.height + kPadding };

////    Mini Mode
const sizei     kmMeterSize     = { kmPadding, 120 };
const sizei     kmButtonSize    = { kmWidth - 2 * kmPadding, kmWidth - kmPadding * 2 };

const sizei     kmSize      = { kmWidth, kmMeterSize.height + kmButtonSize.height * 2 + kmPadding * 5 };

const point2i   kmoMeter    = { kmPadding, kmPadding };
const point2i   kmoButton   = { kmPadding, kmoMeter.y + kmMeterSize.height + kmPadding * 2 };

sizei AudioTrack::_getSize(bool mini) { return mini ? kmSize : kSize; }

////    AudioTrack class    ///////////////////////////////////////////
AudioTrack::AudioTrack(
    awe::Atrack        *source,
    clan::GUIComponent *parent,
    point2i             pos
)   : clan::GUIComponent(parent, "AudioTrack")
    , mTrack(source)
    , m3BEQ (new awe::Filter::Asc3BEQ(
                mTrack->getConfig().targetSampleRate,
                600.0, 8000.0, 1.0, 1.0, 1.0
                ))
    , mMixer(new awe::Filter::AscMixer(
                1.0, 0.0, awe::Filter::AscMixer::IEType::LINEAR
                // Track mixers are linear. Sample mixers are radial
                ))
    , mMeter(new awe::Filter::AscMetering(
                mTrack->getConfig().targetSampleRate / 2,
                1.0))

    , mGCsdvEQGainL(this)
    , mGCsdvEQGainM(this)
    , mGCsdvEQGainH(this)

    , mGCsdhEQFreqL(this)
    , mGCsdhEQFreqH(this)

    , mGCsdvGain(this)
    , mGCsdhPan (this)

    , mGCbtnMute(this)
    , mGCbtnToggleSize(this)
{
    set_geometry(recti{pos, kSize});

    mTrack->getRack().attach_filter(m3BEQ);
    mTrack->getRack().attach_filter(mMixer);
    mTrack->getRack().attach_filter(mMeter);

    ////    EQ GAIN    ////
    mGCsdvEQGainL.set_geometry({ koEQGain, kEQGainSize });
    mGCsdvEQGainM.set_geometry({
            koEQGain.x +     kEQGainSize.width + kPadding / 2,
            koEQGain.y,
            kEQGainSize
            });
    mGCsdvEQGainH.set_geometry({
            koEQGain.x + 2 * kEQGainSize.width + kPadding,
            koEQGain.y,
            kEQGainSize
            });

    mGCsdvEQGainL.set_direction(UI::Slider::Direction::DOWN);
    mGCsdvEQGainM.set_direction(UI::Slider::Direction::DOWN);
    mGCsdvEQGainH.set_direction(UI::Slider::Direction::DOWN);

    mGCsdvEQGainL.set_ranges(-10, 10, 2, 5);
    mGCsdvEQGainM.set_ranges(-10, 10, 2, 5);
    mGCsdvEQGainH.set_ranges(-10, 10, 2, 5);

    mGCsdvEQGainL.set_position(0);
    mGCsdvEQGainM.set_position(0);
    mGCsdvEQGainH.set_position(0);

    mGCsdvEQGainL.func_value_changed().set(this, &AudioTrack::eq_gain_changed);
    mGCsdvEQGainM.func_value_changed().set(this, &AudioTrack::eq_gain_changed);
    mGCsdvEQGainH.func_value_changed().set(this, &AudioTrack::eq_gain_changed);

    mGCsdvEQGainL.set_focus_policy(focus_parent);
    mGCsdvEQGainM.set_focus_policy(focus_parent);
    mGCsdvEQGainH.set_focus_policy(focus_parent);

    ////    EQ FREQ    ////
    mGCsdhEQFreqL.set_geometry({ koEQFreq, kEQFreqSize });
    mGCsdhEQFreqH.set_geometry({
            koEQFreq.x + kEQFreqSize.width,
            koEQFreq.y,
            kEQFreqSize
            });

    mGCsdhEQFreqL.set_direction(UI::Slider::Direction::RIGHT);
    mGCsdhEQFreqH.set_direction(UI::Slider::Direction::RIGHT);

    mGCsdhEQFreqL.set_ranges(0, 20, 2, 5);
    mGCsdhEQFreqH.set_ranges(0, 20, 2, 5);

    mGCsdhEQFreqL.func_value_changed().set(this, &AudioTrack::eq_freq_changed);
    mGCsdhEQFreqH.func_value_changed().set(this, &AudioTrack::eq_freq_changed);

    mGCsdhEQFreqL.set_position(12);
    mGCsdhEQFreqH.set_position(12);

    mGCsdhEQFreqL.set_focus_policy(focus_parent);
    mGCsdhEQFreqH.set_focus_policy(focus_parent);

    ////    PAN    ////
    mGCsdhPan.set_geometry({ koPan, kPanSize });
    mGCsdhPan.set_direction(UI::Slider::Direction::RIGHT);
    mGCsdhPan.set_ranges(-10, 10, 2, 5);
    mGCsdhPan.set_position(0);

    mGCsdhPan.func_value_changed().set(this, &AudioTrack::mixer_value_changed);

    mGCsdhPan.set_focus_policy(focus_parent);

    ////    FADER    ////
    mGCsdvGain.set_geometry({
            get_width() - kPadding - kFaderSize.width,
            koMeter.y,
            kFaderSize
            });
    mGCsdvGain.set_direction(UI::Slider::Direction::DOWN);
    mGCsdvGain.set_ranges(0, 119, 2, 8); // 0 ~ 120; tick step 2; page step 8
    mGCsdvGain.set_position(96);

    mGCsdvGain.func_value_changed().set(this, &AudioTrack::mixer_value_changed);

    mGCsdvGain.set_focus_policy(focus_parent);

    ////    MUTE and TOGGLE MINI MODE BUTTONS    ////
    this->size_toggled(false);

    mGCbtnMute.mcBackground[1] = { 1.0f, 0.6f, 0.6f }; // Red illumination when pressed
    mGCbtnMute.mcBackground[3] = { 1.0f, 0.8f, 1.0f };
    mGCbtnMute.mcBackground[4] = { 1.0f, 0.4f, 0.6f };

    mGCbtnMute      .func_toggled().set(this, &AudioTrack::mute_toggled);
    mGCbtnToggleSize.func_toggled().set(this, &AudioTrack::size_toggled);

    mGCbtnMute      .set_focus_policy(focus_parent);
    mGCbtnToggleSize.set_focus_policy(focus_parent);

    func_render().set(this, &AudioTrack::render);
    set_constant_repaint(true);
}

AudioTrack::~AudioTrack() {
    mTrack->getRack().detach_filter(2);
    mTrack->getRack().detach_filter(1);
    mTrack->getRack().detach_filter(0);
    delete mMeter;
    delete mMixer;
    delete m3BEQ;
}

////    GUI Component Methods    //////////////////////////////////
void AudioTrack::render(clan::Canvas &canvas, const recti &clip_rect)
{
    float mxVol;

    awe::Asintf mtPeak;
    awe::Asintf mtRMS;

    float mScale = 2.0f; // 0 ~ -48dB... at 2px/dB. See UI/FFT.hpp
    float mRange = -awe::dBFS_limit / mScale;

    {   //  Calculate metering
        awe::Asfloatf mtPeakf, mtRMSf;

        std::lock_guard<std::mutex> o_lock(mTrack->getMutex());

        mxVol = mMixer->getVol() * 96.0f;

        mtPeakf = mMeter->getPeak();
        mtRMSf  = mMeter->getAvgRMS();

        mtPeakf([this, mRange, mScale] (float &x) {
                    x = 20.0f * log10(x);
                    x = (x == x) ? ( (x > -mRange) ? (x + mRange) : 0.0f) : 0.0f;
                    x *= mScale;
                });
        mtRMSf([this, mRange, mScale] (float &x) {
                    x = 20.0f * log10(x);
                    x = (x == x) ? ( (x > -mRange) ? (x + mRange) : 0.0f) : 0.0f;
                    x *= mScale;
                });

        mtPeak[0] = mtPeakf[0], mtPeak[1] = mtPeakf[1];
        mtRMS [0] = mtRMSf [0], mtRMS [1] = mtRMSf [1];
    }

    ////    DRAW    ///////////////////////////////////////////////////

    //  Component background
    canvas.fill_rect( recti(0, 0, get_size()), clan::Colorf::white );

    //  Mini mode
    if (this->mGCbtnToggleSize.isOn())
    {
        //  Meter Background
        canvas.fill_rect(
                recti(kmoMeter, sizei(
                    kmWidth             - 2 * kmPadding,
                    kMeterSize.height   + 1 * kmPadding
                )),
                clan::Colorf::black
                );

        canvas.draw_line(
                          kmoMeter.x, kmoMeter.y + 1 + mxVol,
                kmWidth - kmoMeter.x, kmoMeter.y + 1 + mxVol,
                clan::Colorf{ 0.0f, 1.0f, 1.0f, 0.6f }
                );
        canvas.draw_line(
                kmWidth - kmoMeter.x - 1, kmoMeter.y + 1 + mxVol,
                kmWidth - kmoMeter.x - 1, kmoMeter.y + 2 + mxVol,
                clan::Colorf{ 0.0f, 1.0f, 1.0f, 0.6f }
                );
        canvas.draw_line(
                kmWidth - kmoMeter.x, kmoMeter.y + 1 + mxVol,
                kmWidth - kmoMeter.x, kmoMeter.y + 3 + mxVol,
                clan::Colorf{ 0.0f, 1.0f, 1.0f, 0.6f }
                );

        //  Meter Bars
        {
            const int oT = kmoMeter.y + 1;

            const int  w = kmMeterSize.width;
            const int oL = kmoMeter.x + 1;
            const int oR = oL + w + 1;

            auto get_color = [] (awe::Aint const &x) -> clan::Colorf {
                if (x < 64)
                    return clan::Colorf::green;
                if (x < 96)
                    return clan::Colorf::orange;

                return clan::Colorf::red;
            };

            for(awe::Aint p = 0; p < 120; p += w+1)
            {
                clan::Colorf cRMS  = get_color(p);
                clan::Colorf cPeak = { cRMS.r * 0.6f, cRMS.g * 0.6f, cRMS.b * 0.6f };
                clan::Colorf cNone = { 0.1f, 0.1f, 0.1f };

                /**/ if (mtRMS [0] > p)
                    canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cRMS);
                else if (mtPeak[0] > p)
                    canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cPeak);
                else
                    canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cNone);

                /**/ if (mtRMS [1] > p)
                    canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cRMS);
                else if (mtPeak[1] > p)
                    canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cPeak);
                else
                    canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cNone);
            }

            // TODO Implement audio overclip indicator
        }

        return;
    }

    //  Large mode

    //  Draw component split line
    canvas.draw_line( 1, kySplit, get_width() - 1, kySplit, { 0.8f, 0.8f, 0.8f } );

    //  Meter Background
    canvas.fill_rect(
            recti(
                koMeter,
                sizei(
                    2 * kPadding + 2 * kMeterSize.width + 1,
                    2 * kPadding + kMeterSize.height
                )),
            clan::Colorf::black
            );

    //  Gain line
    canvas.draw_line(
            koMeter.x + kmPadding                                      , koMeter.y + kPadding + mxVol,
            koMeter.x + kmPadding + kPadding + kMeterSize.width * 2 + 1, koMeter.y + kPadding + mxVol,
            clan::Colorf{ 0.0f, 1.0f, 1.0f, 0.6f }
            );

    //  Meter Bars
    {
        const int oT = kPadding + koMeter.y;

        const int w  = kMeterSize.width;
        const int oL = koMeter.x + kPadding;
        const int oR = oL + w + 1;

        auto get_color = [] (awe::Aint const &x) -> clan::Colorf {
            if (x < 64)
                return clan::Colorf::green;
            if (x < 96)
                return clan::Colorf::orange;

            return clan::Colorf::red;
        };

        for(awe::Aint p = 0; p < 120; p += w+1)
        {
            clan::Colorf cRMS  = get_color(p);
            clan::Colorf cPeak = { cRMS.r * 0.6f, cRMS.g * 0.6f, cRMS.b * 0.6f };
            clan::Colorf cNone = { 0.1f, 0.1f, 0.1f };

            /**/ if (mtRMS [0] > p)
                canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cRMS);
            else if (mtPeak[0] > p)
                canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cPeak);
            else
                canvas.fill_rect(oL, oT+p, oL+w, oT+p+w, cNone);

            /**/ if (mtRMS [1] > p)
                canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cRMS);
            else if (mtPeak[1] > p)
                canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cPeak);
            else
                canvas.fill_rect(oR, oT+p, oR+w, oT+p+w, cNone);
        }

        // TODO Implement audio overclip indicator
    }

    //  Markers for Meter bar and Fader
    {
        // draw dB lines, y + 1 because of dBFS round up
        int o = 4 * kPadding + 2 * kMeterSize.width + 1;
        int t = 1 + koMeter.y + kPadding;

        for(int i = t; i < t + kMeterSize.height; i += 16)
            canvas.draw_line(
                    o               , i,
                    o + kMarkerWidth, i,
                    clan::Colorf::grey
                    );

        // 0dBFS full line
        canvas.draw_line(
                o               , t + 96,
                o + kMarkerWidth, t + 96,
                clan::Colorf::black
                );
    }
}

void AudioTrack::eq_gain_changed()
{
    float l = mGCsdvEQGainL.get_position(); l = l / 10.0f + 1.0f; // -10 ~ 10 -> 0.0f -> 2.0f
    float m = mGCsdvEQGainM.get_position(); m = m / 10.0f + 1.0f;
    float h = mGCsdvEQGainH.get_position(); h = h / 10.0f + 1.0f;

    m3BEQ->set_gain(l,m,h);
}

void AudioTrack::eq_freq_changed()
{
    float l = mGCsdhEQFreqL.get_position(); l = 2.0f * pow(10.0f, 1.0 + l / 10.0f); // 0 ~ 20 ->  200 ~  2000Hz
    float h = mGCsdhEQFreqH.get_position(); h = 2.0f * pow(10.0f, 2.0 + h / 10.0f); // 0 ~ 20 -> 2000 ~ 20000Hz

    clan::Console::write_line("LPF = %1, HPF = %2, SR = %3", l, h, mTrack->getConfig().targetSampleRate);
    m3BEQ->set_freq(l,h);
}

void AudioTrack::mixer_value_changed()
{
    float x = -mGCsdhPan.get_position(); x = x / 10.0f;
    float y = mGCsdvGain.get_position(); y = y / 20.0f / -log10(awe::Afloat_96dB);

    mMixer->setPan(x);
    mMixer->setVol(y);
}

void AudioTrack::mute_toggled(bool mute)
{
    clan::Console::write_line("Toggling to %1", mute ? "MUTE" : "UNMUTE");
    awe::ArenderConfig config = mTrack->getConfig();
    config.quality = mute ? awe::ArenderConfig::Quality::MUTE : awe::ArenderConfig::Quality::DEFAULT;
    mTrack->setConfig(config);
}

void AudioTrack::size_toggled(bool mini)
{
    //  Toggle controller visibility
    mGCsdvEQGainL.set_visible(!mini);
    mGCsdvEQGainM.set_visible(!mini);
    mGCsdvEQGainH.set_visible(!mini);
    mGCsdhEQFreqL.set_visible(!mini);
    mGCsdhEQFreqH.set_visible(!mini);

    mGCsdhPan .set_visible(!mini);
    mGCsdvGain.set_visible(!mini);

    // Resize elements
    if (mini) {
        set_geometry(recti{ get_geometry().get_top_left(), kmSize });

        mGCbtnMute      .set_geometry(recti{ kmoButton, kmButtonSize });
        mGCbtnToggleSize.set_geometry(recti{ kmoButton.x, kmoButton.y + kmButtonSize.height + kmPadding, kmButtonSize });
    } else {
        set_geometry(recti{ get_geometry().get_top_left(), kSize });

        mGCbtnMute      .set_geometry({ koButton, kButtonSize });
        mGCbtnToggleSize.set_geometry({ get_width() - kPadding - kButtonSize.width, koButton.y, kButtonSize });
    }
}
