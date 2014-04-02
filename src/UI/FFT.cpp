#include "FFT.hpp"

namespace UI {

inline const /* constexpr */ double _sinc(double const &x) { return sin(M_PI * x) / (M_PI * x); }
static const /* constexpr */ double _lanczos_size = 3.0f;
inline const /* constexpr */ double _lanczos_sinc(double const &x)
{
    return
        (x == 0.0)          ? 1.0 : (
        (x > _lanczos_size) ? 0.0 : (
            _sinc(x) * _sinc(x / _lanczos_size)
        )
        );
}

std::vector<float> FFT::generate_window(FFT::IEWindowType const &type, ulong const &size)
{
    std::vector<float> window(size);

    switch (type)
    {
        case FFT::IEWindowType::LANCZOS:
            for(ulong t = 0; t < size; ++t)
                window[t] = _lanczos_sinc(static_cast<double>(2 * t) / static_cast<double>(size - 1) - 1.0f);
            break;

        case FFT::IEWindowType::HANNING:
            for(ulong t = 0; t < size; ++t)
                window[t] = 0.5 * (1.0 - cos( 2.0 * M_PI * static_cast<double>(t) / static_cast<double>(size - 1)));
            break;

        case FFT::IEWindowType::RECTANGULAR:
            for(ulong t = 0; t < size; ++t)
                window[t] = 1.0f;
            break;
    }

    return window;
}


FFT::FFT(
    clan::GUIComponent *parent,
    recti area,
    ulong frames,
    ulong sample_rate,
    ulong bands,
    float fade,
    IEWindowType window,
    IEScaleType  scale
)   : clan::GUIComponent(parent, "FFT")
    , mFrames       (frames - (frames % 2))
    , mFade         (fade)
    , mWindowType   (window)
    , mScaleType    (scale)
    , mScale        (1.0f)
    , mRange        (-awe::dBFS_limit / mScale)
    , mOutput       (2, mFrames)
    , mSpectrum     (2, 0)
{
    set_geometry(area);

    setBands(bands);
    setWindow(window);
    setSampleRate(sample_rate);

    kCl = kiss_fftr_alloc(mFrames, 0, NULL, NULL);
    kCr = kiss_fftr_alloc(mFrames, 0, NULL, NULL);

    kIl = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));
    kIr = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));

    kOl = (kiss_fft_cpx*)calloc(mFrames, sizeof(kiss_fft_cpx));
    kOr = (kiss_fft_cpx*)calloc(mFrames, sizeof(kiss_fft_cpx));

    set_focus_policy(clan::GUIComponent::FocusPolicy::focus_refuse);

    func_render().set(this, &FFT::render);
}

FFT::~FFT ()
{
    kiss_fftr_free(kCl);
    kiss_fftr_free(kCr);

    free(kIl);
    free(kIr);
    free(kOl);
    free(kOr);
}

void FFT::setSampleRate(ulong rate)
{
    mDecay =
        mFade * (static_cast<float>(mFrames) / static_cast<float>(rate));
}

void FFT::setBands(ulong n)
{
    mBands = std::min(mFrames, n);

    mBandX.clear();
    mBandX.reserve(1+mBands);
    mBandX.resize (1+mBands);

    mSpectrum.vector().clear();
    mSpectrum.vector().reserve(2*mBands);
    mSpectrum.vector().resize (2*mBands);

    for(ulong i = 0; i <= mBands; ++i)
        mBandX[i] = pow(
            static_cast<float>(mFrames / 2),
            static_cast<float>(i) / static_cast<float>(mBands)
            ) - 0.5f;
}

void FFT::setWindow(FFT::IEWindowType type)
{
    std::vector<awe::Afloat> window = generate_window(type, mFrames);
    mWindow.swap(window);

    // Calculate window scaling
    for (ulong t = 0; t < mFrames; ++t)
        mWindowScale += mWindow[t];

    mWindowScale = (mWindowScale > 0.0f) ? 4.0f / (mWindowScale * mWindowScale) : 1.0f;
}


void FFT::calc_FFT(const awe::AfBuffer& buffer)
{
    /* Read input and apply window */
    for (ulong t = 0; t < mFrames; t++)
    {
        kIl[t] = buffer.getSample(t*2)   * mWindow[t];
        kIr[t] = buffer.getSample(t*2+1) * mWindow[t];
    }

    kiss_fftr( kCl , kIl , kOl );
    kiss_fftr( kCr , kIr , kOr );

    ulong const n = mFrames / 2; /* Look for: Nyquist Theorem */

    /* Get magnitude of FFT */
    for (ulong t = 0; t < n; ++t)
    {
        float rl, il, rr, ir;

        rl = kOl[t].r;
        il = kOl[t].i;

        rr = kOr[t].r;
        ir = kOr[t].i;

        mOutput.data()[t*2  ] = rl*rl + il*il;
        mOutput.data()[t*2+1] = rr*rr + ir*ir;

    }
}


void FFT::update(awe::AfBuffer const& buffer)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (is_enabled() == false)
        return;

    calc_FFT(buffer);

    for(ulong i = 0; i < mBands; i++)
    {
        unsigned a = ceil (mBandX[i  ]);
        unsigned b = floor(mBandX[i+1]);
        unsigned c = a - 1;

        awe::Asfloatf n({ 0.0f, 0.0f });
        awe::Asfloatf o(mSpectrum.getFrame(i));

        if (b < a) {
            n[0] += mOutput.getSample(b*2  ) * (mBandX[i+1] - mBandX[i]),
            n[1] += mOutput.getSample(b*2+1) * (mBandX[i+1] - mBandX[i]);
        } else {
            if (a > 0)
                n[0] += mOutput.getSample(c*2  ) * (a - mBandX[i]),
                n[1] += mOutput.getSample(c*2+1) * (a - mBandX[i]);

            for (; a < b; a++)
                n[0] += mOutput.getSample(a*2  ),
                n[1] += mOutput.getSample(a*2+1);

            if (b < mBands)
                n[0] += mOutput.getSample(b*2  ) * (mBandX[i+1] - b),
                n[1] += mOutput.getSample(b*2+1) * (mBandX[i+1] - b);
        }

        switch (mScaleType)
        {
            case FFT::IEScaleType::DBFS:
                n( [this] (float &x) {
                        x = 10.0f * log10(x * mWindowScale);
                        x = (x == x) ? ( (x > -mRange) ? (x + mRange) : 0.0f ) : 0.0f;
                        } );
                break;

            case FFT::IEScaleType::CUBIC:
                n( [] (float &x) { x = powf(x, 1.0f/3.0f); });
                n *= mRange;
                break;

            case FFT::IEScaleType::LINEAR:
                n( [] (float &x) { x = sqrt(x); });
                n *= mRange * 2.0f;
                break;
        }

        o[0] -= mRange * mDecay,
        o[1] -= mRange * mDecay;

        o[0] = o[0] > 0.0f ? o[0] : 0.0f,
        o[1] = o[1] > 0.0f ? o[1] : 0.0f;

        n[0] = n[0] > o[0] ? n[0] : o[0],
        n[1] = n[1] > o[1] ? n[1] : o[1];

        mSpectrum.setFrame(i, n.data.data());
    }
}


////    GUI Component Callbacks    ////////////////////////////////////
void FFT::render(clan::Canvas &canvas, recti const &clip_rect)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (is_enabled() == false)
        return;

    float bandw = static_cast<float>(get_height()) / static_cast<float>(mBands);
    float midpt = static_cast<float>(get_width ()) / 2.0f;

    // Draw metering lines
    canvas.draw_line(
        midpt, 0, midpt, get_height(),
        clan::Colorf(1.0f, 1.0f, 1.0f, 0.05f)
        );

    const uchar n = 4;
    const float l = awe::dBFS_limit / n;

    for (int i = 1; i <= 4; ++i)
    {
        float j = static_cast<float>(i) * l;
        canvas.draw_line(
            midpt - j, 0, midpt - j, get_height(),
            clan::Colorf(1.0f, 1.0f, 1.0f, 0.01f)
            );
        canvas.draw_line(
            midpt + j, 0, midpt + j, get_height(),
            clan::Colorf(1.0f, 1.0f, 1.0f, 0.01f)
            );
    }

    // Draw spectrum
    clan::ColorHSVf color(0.0f, 1.0f, 1.0f, 0.1f);
    float inc = 270.0f / mBands;

    for(ulong i = 0; i < mBands; ++i)
    {
        color.h += inc;
        rectf out {
            midpt - mSpectrum.getSample(i*2  ), bandw * (mBands-i  ),
            midpt + mSpectrum.getSample(i*2+1), bandw * (mBands-i+1)
        };

        canvas.fill_rect(out, color);
    }
}


}
