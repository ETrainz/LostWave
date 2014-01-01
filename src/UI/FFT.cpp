//  UI/FFT.cpp :: FFT visualizer
//  Copyright 2013 Keigen Shu

#include "FFT.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

namespace UI {

constexpr double _sinc(double const &x) { return sin(M_PI * x) / (M_PI * x); }
constexpr double _lanczos_size = 3.0f;
constexpr double _lanczos_sinc(double const &x)
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
                window[t] = _lanczos_sinc(static_cast<double>(2 * t) / static_cast<double>(size - 1) - 1.0);
            break;

        case FFT::IEWindowType::HANNING:
            for(ulong t = 0; t < size; ++t)
                window[t] = 0.5f * (1.0f - cos( 2.0f * M_PI * float(t) / (float(size) - 1.0f)));
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
) : clan::GUIComponent(parent, { area, false }, "FFT"),
    mArea       (area),
    mFrames     (frames - (frames % 2)),
    mFade       (fade),
    mWindowType (window),
    mScaleType  (scale),
    mWindow     (generate_window(window, mFrames)),
    mOutput     (2, mFrames),
    mSpectrum   (2, bands),
    mScale      (1.0f), // ~ -64.22dB to -inf is ignored.
    mRange      (-awe::dBFS_limit / mScale),
    mBands      (0),
    mBandX      (nullptr),
    mRects      (nullptr)
{
    setSampleRate(sample_rate);

    kCl = kiss_fftr_alloc(mFrames, 0, NULL, NULL);
    kCr = kiss_fftr_alloc(mFrames, 0, NULL, NULL);

    kIl = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));
    kIr = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));

    kOl = (kiss_fft_cpx*)calloc(mFrames, sizeof(kiss_fft_cpx));
    kOr = (kiss_fft_cpx*)calloc(mFrames, sizeof(kiss_fft_cpx));

    // Calculate band translation table
    set_bands(bands);

    func_render().set(this, &FFT::render);
}

FFT::~FFT ()
{
    if (mBandX != nullptr)
        delete[] mBandX;

    kiss_fftr_free(kCl);
    kiss_fftr_free(kCr);

    free(kIl);
    free(kIr);
    free(kOl);
    free(kOr);
}

void FFT::setSampleRate(ulong rate)
{
    mDecayPeriod =
        mFade * (static_cast<float>(mFrames) / static_cast<float>(rate));
}

void FFT::set_bands(ulong n)
{
    if (mBandX != nullptr)
        delete[] mBandX;

    mBands = n;
    mBandX = new float[1 + n];
    mSpectrum.reset();

    for(ulong i = 0; i <= n; ++i)
        mBandX[i] = powf(
                        static_cast<float>(mFrames / 2),
                        static_cast<float>(i) / static_cast<float>(n)
                    ) - 0.5f;

    calc_rects();
}

void FFT::calc_rects()
{
    float bandw = static_cast<float>(mArea.get_height()) / static_cast<float>(mBands);
    float midpt = static_cast<float>(mArea.left) + static_cast<float>(mArea.get_width()) / 2.0f;

    if (mRects != nullptr)
        delete[] mRects;

    mRects = new rectf[mBands]();

    for(int i = 0; i < mBands; ++i)
        mRects[i] = rectf(
                        midpt, bandw * (mBands-i-1),
                        midpt, bandw * (mBands-i)
                    );
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

        mOutput.data()[t*2  ] = powf((rl*rl + il*il) / (n*n), 0.5f);
        mOutput.data()[t*2+1] = powf((rr*rr + ir*ir) / (n*n), 0.5f);
    }
}


void FFT::update(awe::AfBuffer const& buffer)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (is_enabled())
    {
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
                            x = 20.0f * log10(x);
                            x = (x == x) ? ( (x > -mRange) ? (x + mRange) : 0.0 ) : 0.0;
                            } );
                    break;

                case FFT::IEScaleType::CUBIC:
                    n( [] (float &x) { x = powf(x, 1.0f/3.0f); });
                    n *= mRange;
                    break;

                case FFT::IEScaleType::LINEAR:
                    n *= mRange * 2.0f;
                    break;
            }

            o[0] -= mRange * mDecayPeriod,
            o[1] -= mRange * mDecayPeriod;

            o[0] = o[0] > 0.0f ? o[0] : 0.0f,
            o[1] = o[1] > 0.0f ? o[1] : 0.0f;

            n[0] = n[0] > o[0] ? n[0] : o[0],
            n[1] = n[1] > o[1] ? n[1] : o[1];

            mSpectrum.setFrame(i, n.data.data());
        }
    }
}


////    GUI Component Callbacks    ////////////////////////////////////
void FFT::render(clan::Canvas &canvas, recti const &clip_rect)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (is_enabled())
    {
        // Draw metering lines
        canvas.draw_line(
            canvas.get_width() / 2, 0,
            canvas.get_width() / 2, canvas.get_height(),
            clan::Colorf(1.0f, 1.0f, 1.0f, 0.05f)
        );

        for(int i = 1; i <= 5; ++i)
        {
            canvas.draw_line(
                canvas.get_width() / 2 - (i * 24.0f), 0,
                canvas.get_width() / 2 - (i * 24.0f), canvas.get_height(),
                clan::Colorf(1.0f, 1.0f, 1.0f, 0.01f)
            );
            canvas.draw_line(
                canvas.get_width() / 2 + (i * 24.0f), 0,
                canvas.get_width() / 2 + (i * 24.0f), canvas.get_height(),
                clan::Colorf(1.0f, 1.0f, 1.0f, 0.01f)
            );
        }

        // Draw spectrum
        clan::ColorHSVf color(0.0f, 1.0f, 1.0f, 0.1f);
        float inc = 270.0f / mBands;

        for(ulong i = 0; i < mBands; ++i)
        {
            color.h   += inc;
            rectf out  = mRects[i];
            out.left  -= mSpectrum.getSample(i*2  );
            out.right += mSpectrum.getSample(i*2+1);
            canvas.fill_rect(out, color);
        }
    }
}


}
