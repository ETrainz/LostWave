//  UI/FFT.cpp :: FFT visualizer
//  Copyright 2013 Keigen Shu

#include "FFT.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

namespace UI {

FFT::FFT(
    clan::GUIComponent *parent,
    recti area,
    ulong frames,
    ulong sample_rate,
    ulong bands,
    float fade
) : clan::GUIComponent(parent, { area, false }, "FFT"),
    mFrames     (frames - (frames % 2)),
    mFade       (fade),
    mBandX      (nullptr),
    mSpectrum   (nullptr),
    mRects      (nullptr),
    mArea       (area)
{
    setSampleRate(sample_rate);

    kCl = kiss_fftr_alloc(mFrames, 0, NULL, NULL);
    kCr = kiss_fftr_alloc(mFrames, 0, NULL, NULL);

    kIl = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));
    kIr = (kiss_fft_scalar*)calloc(mFrames, sizeof(kiss_fft_scalar));

    kOl = (kiss_fft_cpx*)calloc(mFrames, sizeof(kiss_fft_cpx));
    kOr = (kiss_fft_cpx*)calloc(frames, sizeof(kiss_fft_cpx));

    mOutput = (float*)calloc(mFrames, sizeof(float));

    // Generate FFT window
    mWindow = (float*)calloc(mFrames, sizeof(float));
    for (ulong t = 0; t < mFrames; ++t)
    {
        // Lanczos sinc
        // window[t] = sin(M_PI * (2.0f * float(t) / (float(frames - 1) - 1.0f));
        // window[t] = sin(a) / a;

        // Hanning
        mWindow[t]  = 1.0f - cos( 2.0f * M_PI * float(t) / (float(mFrames) - 1.0f));
        mWindow[t] *= 0.5f;
    }

    // Calculate band translation table
    set_bands(bands);

    func_render().set(this, &FFT::render);
}

FFT::~FFT ()
{
    if (mBandX != nullptr)
        delete[] mBandX;
    if (mSpectrum != nullptr)
        delete[] mSpectrum;

    kiss_fftr_free(kCl);
    kiss_fftr_free(kCr);

    free(mOutput);
    free(mWindow);
    free(kIl);
    free(kIr);
    free(kOl);
    free(kOr);
}

void FFT::setSampleRate(ulong rate)
{
    mDecayPeriod =
        (2.0f * static_cast<float>(mFrames))
        / (mFade * static_cast<float>(rate));

    mFudgeFrames =
        std::ceil(static_cast<float>(rate) / static_cast<float>(mFrames))
        / 18.0f;
}

void FFT::set_bands(ulong n)
{
    if (mBandX != nullptr)
        delete[] mBandX;
    if (mSpectrum != nullptr)
        delete[] mSpectrum;

    mBands       = n;
    mBandX       = new float[1 + n];
    mSpectrum    = new float[2 * mBands]();

    for(ulong i = 0; i <= n; ++i)
        mBandX[i] = powf(
                        static_cast<float>(mFrames / 2 - mFudgeFrames),
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

    ulong const actualFrames = mFrames / 2; /* Look for: Nyquist Theorem */

    /* Get magnitude of FFT */
    for (ulong t = 0; t < actualFrames; ++t)
    {
        float rl, il, rr, ir;

        rl = kOl[t].r;
        il = kOl[t].i;

        rr = kOr[t].r;
        ir = kOr[t].i;

        // output[t*2  ] = pow(rl*rl + il*il, 1.0/3.0);
        // output[t*2+1] = pow(rr*rr + ir*ir, 1.0/3.0);
        // output[t*2  ] = 10.0f * log10(rl*rl + il*il);
        // output[t*2+1] = 10.0f * log10(rr*rr + ir*ir);

        mOutput[t*2  ] = rl * rl + il * il;
        mOutput[t*2+1] = rr * rr + ir * ir;
    }
}


void FFT::update(awe::AfBuffer const& buffer)
{
    std::lock_guard<std::mutex> lock(mMutex);

    calc_FFT(buffer);

    for(ulong i = 0; i < mBands; i++)
    {
        unsigned a = ceil (mBandX[i  ]);
        unsigned b = floor(mBandX[i+1]);
        unsigned c = a + 1 + mFudgeFrames;
        unsigned d = b + 1 + mFudgeFrames;

        float nl = 0, nr = 0;

        if (b < a) {
            nl += mOutput[d*2  ] * (mBandX[i+1] - mBandX[i]),
                  nr += mOutput[d*2+1] * (mBandX[i+1] - mBandX[i]);
        } else {
            if (a > 0)
                nl += mOutput[(c-1)*2  ] * (a - mBandX[i]),
                      nr += mOutput[(c  )*2-1] * (a - mBandX[i]);

            for (; a < b; a++)
                nl += mOutput[c*2  ],
                      nr += mOutput[c*2+1];

            if (b < mBands)
                nl += mOutput[d*2  ] * (mBandX[i+1] - b),
                      nr += mOutput[d*2+1] * (mBandX[i+1] - b);
        }

        if (nl > 0)
            nl = powf(nl, 1.0f/3.0f) * 48.0f;
        else
            nl = 0.0f;

        if (nr > 0)
            nr = powf(nr, 1.0f/3.0f) * 48.0f;
        else
            nr = 0.0f;

        float Ol = mSpectrum[i*2  ];
        float Or = mSpectrum[i*2+1];
        mSpectrum[i*2  ] = Ol - Ol * mDecayPeriod + nl * mDecayPeriod;
        mSpectrum[i*2+1] = Or - Or * mDecayPeriod + nr * mDecayPeriod;
    }
}


////    GUI Component Callbacks    ////////////////////////////////////
void FFT::render(clan::Canvas &canvas, recti const &clip_rect)
{
    std::lock_guard<std::mutex> lock(mMutex);

    clan::ColorHSVf color(0.0f, 1.0f, 1.0f, 0.1f);
    float inc = 270.0f / mBands;
    for(ulong i = 0; i < mBands; ++i)
    {
        color.h += inc;
        rectf out  = mRects[i];
        out.left  -= mSpectrum[i*2  ];
        out.right += mSpectrum[i*2+1];
        canvas.fill_rect(out, color);
    }
}


}
