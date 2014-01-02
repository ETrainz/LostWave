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
public:
    enum class IEWindowType : uint8_t
    {
        HANNING     = 'H',
        LANCZOS     = 'L',
        RECTANGULAR = 'R'
    };

    enum class IEScaleType : uint8_t
    {
        LINEAR = 'L',
        DBFS   = 'D',
        CUBIC  = 'C'
    };

    static std::vector<float> generate_window(IEWindowType const &window, ulong const &size);

private:
    std::mutex      mMutex;     //! Master mutex

    recti           mArea;      //! display area
    ulong           mFrames;    //! number of frames to process
    ulong           mBands;     //! number of bands to show
    float           mFade;      //! decay strength

    IEWindowType    mWindowType;
    IEScaleType     mScaleType;

    kiss_fftr_cfg    kCl,  kCr; // KISSFFT config
    kiss_fft_scalar *kIl, *kIr; // input
    kiss_fft_cpx    *kOl, *kOr; // output

protected:
    std::vector<float>  mWindow;    // FFT window function coefficients
    awe::AfBuffer       mOutput;
    awe::AfBuffer       mSpectrum;  // output spectrum to be drawn

    float           mDecayPeriod;   // decay over time
    float           mScale;         // range of values to display (for log scale from 0db to x)
    float           mRange;         // number of pixels
    float           mMultiplier;    // the bands peak at around -5 dBFS, 

    float*          mBandX;         // linear to logarithmic table
    rectf*          mRects;

    void set_bands(ulong);
    void calc_rects();

public:
    FFT(clan::GUIComponent *parent,
        recti area,
        ulong frames,
        ulong sample_rate,
        ulong bands,
        float fade = 0.1f,
        IEWindowType window = IEWindowType::HANNING,
        IEScaleType  scale  = IEScaleType::DBFS
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
