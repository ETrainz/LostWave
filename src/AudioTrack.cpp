#include "AudioTrack.hpp"

const int thumb_height = 6; // Refer to theme CSS

const int   AudioTrack::kPadding         = 4;

const sizei AudioTrack::kEQGainSize      = {
    static_cast<int>(static_cast<float>(40 - 3 * kPadding) / 3.0f),
    41
};

const sizei AudioTrack::kEQFreqSize        = {
    static_cast<int>(static_cast<float>(40 - 2 * kPadding) / 2.0f),
    9
};

const sizei AudioTrack::kPanBarSize      = { 40 - kPadding * 2, 9 };
const sizei AudioTrack::kMeterBarSize    = { kPadding, 120 };
const int   AudioTrack::kMarkerWidth     = 5;
const sizei AudioTrack::kFaderSize       = { 12, 120 };

const sizei AudioTrack::kSize            = { 40, kEQGainSize.height + kEQFreqSize.height + kPanBarSize.height + kMeterBarSize.height + 9 * kPadding };

const point2i koEQGainBars  = { AudioTrack::kPadding, AudioTrack::kPadding };
const point2i koEQFreqBars  = { AudioTrack::kPadding, koEQGainBars.y + AudioTrack::kEQGainSize.height + AudioTrack::kPadding };
const int     kySplit       = koEQFreqBars.y + AudioTrack::kEQFreqSize.height + AudioTrack::kPadding * 2;
const point2i koPanBars     = { AudioTrack::kPadding, AudioTrack::kPadding + kySplit };
const point2i koMeterBars   = { AudioTrack::kPadding, koPanBars.y + AudioTrack::kPanBarSize.height + AudioTrack::kPadding };

AudioTrack::AudioTrack(
    awe::Atrack        *source,
    clan::GUIComponent *parent,
    point2i             pos
)   : clan::GUIComponent(parent, { { pos, kSize }, false }, "AudioTrack")
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
{
    mTrack->getRack().attach_filter(m3BEQ);
    mTrack->getRack().attach_filter(mMixer);
    mTrack->getRack().attach_filter(mMeter);

    ////    EQ GAIN    ////
    mGCsdvEQGainL.set_geometry({ koEQGainBars, kEQGainSize });
    mGCsdvEQGainM.set_geometry({
            koEQGainBars.x +     kEQGainSize.width + kPadding / 2,
            koEQGainBars.y,
            kEQGainSize
            });
    mGCsdvEQGainH.set_geometry({
            koEQGainBars.x + 2 * kEQGainSize.width + kPadding,
            koEQGainBars.y,
            kEQGainSize
            });

    mGCsdvEQGainL.set_vertical(true);
    mGCsdvEQGainM.set_vertical(true);
    mGCsdvEQGainH.set_vertical(true);

    mGCsdvEQGainL.set_ranges(-10, 10, 2, 5);
    mGCsdvEQGainM.set_ranges(-10, 10, 2, 5);
    mGCsdvEQGainH.set_ranges(-10, 10, 2, 5);

    mGCsdvEQGainL.set_position(0);
    mGCsdvEQGainM.set_position(0);
    mGCsdvEQGainH.set_position(0);

    mGCsdvEQGainL.func_value_changed().set(this, &AudioTrack::eq_gain_changed);
    mGCsdvEQGainM.func_value_changed().set(this, &AudioTrack::eq_gain_changed);
    mGCsdvEQGainH.func_value_changed().set(this, &AudioTrack::eq_gain_changed);

    ////    EQ FREQ    ////

    mGCsdhEQFreqL.set_geometry({ koEQFreqBars, kEQFreqSize });
    mGCsdhEQFreqH.set_geometry({
            koEQFreqBars.x + kEQFreqSize.width,
            koEQFreqBars.y,
            kEQFreqSize
            });

    mGCsdhEQFreqL.set_horizontal(true);
    mGCsdhEQFreqH.set_horizontal(true);

    mGCsdhEQFreqL.set_ranges(0, 20, 2, 5);
    mGCsdhEQFreqH.set_ranges(0, 20, 2, 5);

    mGCsdhEQFreqL.func_value_changed().set(this, &AudioTrack::eq_freq_changed);
    mGCsdhEQFreqH.func_value_changed().set(this, &AudioTrack::eq_freq_changed);

    mGCsdhEQFreqL.set_position(12);
    mGCsdhEQFreqH.set_position(12);

    ////    PAN    ////
    mGCsdhPan.set_geometry({ koPanBars, kPanBarSize });
    mGCsdhPan.set_horizontal(true);
    mGCsdhPan.set_ranges(-10, 10, 2, 5);
    mGCsdhPan.set_position(0);

    mGCsdhPan.func_value_changed().set(this, &AudioTrack::mixer_value_changed);

    ////    FADER    ////
    int range = kFaderSize.height - thumb_height;

    mGCsdvGain.set_geometry({
            get_width() - kPadding - kFaderSize.width,
            koMeterBars.y + kPadding,
            kFaderSize
            });
    mGCsdvGain.set_vertical(true);
    mGCsdvGain.set_ranges(0, range, 2, 8); // 0 ~ 120; tick step 2; page step 8
    mGCsdvGain.set_position(96);

    mGCsdvGain.func_value_changed().set(this, &AudioTrack::mixer_value_changed);



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
    double eqLG, eqMG, eqHG;    // Band gain
    double eqLF, eqHF;          // Band window

    float mxVol, mxPan;

    awe::Asintf mtPeak;
    awe::Asintf mtRMS;

    float mScale = 2.0f; // 0 ~ -48dB... at 2px/dB. See UI/FFT.hpp
    float mRange = -awe::dBFS_limit / mScale;

    {
        awe::Asfloatf mtPeakf, mtRMSf;

        std::lock_guard<std::mutex> o_lock(mTrack->getMutex());
        m3BEQ->get_freq(eqLF, eqHF);
        m3BEQ->get_gain(eqLG, eqMG, eqHG);
        mxVol  = mMixer->getVol();
        mxPan  = mMixer->getPan();

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

    //  Component background and split line
    canvas.fill_rect( recti(0, 0, get_size()), clan::Colorf::white );
    canvas.draw_line( 1, kySplit, get_width() - 1, kySplit, { 0.8f, 0.8f, 0.8f } );

    //  Meter Background
    canvas.fill_rect(
            recti(
                kPadding, koMeterBars.y,
                sizei(
                    2 * kPadding + 2 * kMeterBarSize.width + 1,
                    2 * kPadding + kMeterBarSize.height
                )),
            clan::Colorf::black
            );

    //  Meter Bars
    {
        const int oT = kPadding + koMeterBars.y;

        const int w = kMeterBarSize.width;
        const int oL = koMeterBars.x + kPadding;
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
            clan::Colorf cPeak = cRMS; cPeak.a = 0.6f;
            clan::Colorf cNone = clan::Colorf::white; cNone.a = 0.1f;

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
        int o = 4 * kPadding + 2 * kMeterBarSize.width + 1;
        int t = koMeterBars.y + kPadding;

        for(int i = 1; i < get_height(); i += 16)
            canvas.draw_line(
                    o               , t + i,
                    o + kMarkerWidth, t + i,
                    clan::Colorf::grey
                    );

        // 0dBFS full line
        canvas.draw_line(
                o               , t + 96 + 1,
                o + kMarkerWidth, t + 96 + 1,
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
