//  UI/FFT.hpp :: FFT visualizer
//  Copyright 2013 Keigen Shu

#ifndef UI_FFT_H
#define UI_FFT_H

#include "../__zzCore.hpp"
#include "../libawe/aweBuffer.h"

#include "../kiss_fft130/kiss_fft.h"
#include "../kiss_fft130/kiss_fftr.h"

#include <mutex>

namespace UI {

class FFT : public clan::GUIComponent
{
private:
    std::mutex      mMutex; /// Master mutex

    kiss_fftr_cfg    kCl,  kCr; // KISSFFT config
    kiss_fft_scalar *kIl, *kIr; // input
    kiss_fft_cpx    *kOl, *kOr; // output

    float*          mWindow;    // FFT window function coefficients
    float*          mOutput;

    ulong const     mFrames;
    float           mFade;   // decay strength
    recti           mArea;   // display area

protected:
    float   mDecayPeriod;   // decay over time

    ulong   mBands;     // number of bands to show
    float*  mBandX;     // linear to logarithmic table
    float*  mSpectrum;  // output spectrum to be drawn

    rectf*  mRects;

    void set_bands(ulong);
    void calc_rects();

public:
    FFT(clan::GUIComponent *parent,
        recti area,
        ulong frames,
        ulong sample_rate,
        ulong bands,
        float fade = 0.1f
       );
    ~FFT();

    inline std::mutex &getMutex() { return mMutex; }

    void setSampleRate(ulong rate);

    void calc_FFT(awe::AfBuffer const &buffer);
    void update(awe::AfBuffer const &buffer);

    ////    GUI Component Callbacks    ////////////////////////////////
    void render(clan::Canvas &canvas, recti const &clip_rect);
};

}

#endif
