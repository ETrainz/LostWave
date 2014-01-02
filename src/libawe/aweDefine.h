//  aweDefine.h :: Essential declarations and definitions
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_DEFINE
#define AWE_DEFINE

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#include <algorithm>
#include <array>
#include <queue>

namespace awe {

/* Basic datatypes */
using Aint   = int16_t;
using Afloat = float;

/* FIFO buffer */
typedef std::queue<Afloat> AfFIFOBuffer;
typedef std::queue<Aint  > AiFIFOBuffer;

#define IO_BUFFER_SIZE  16384   /** File IO buffer size */

/** @return Afloat equivalent of Aint */
inline const /* constexpr */ Afloat to_Afloat(const Aint &v)
{
    if (v == 0)
        return 0;
    else if (v < 0)
        return float(v) / 32768.0f;
    else
        return float(v) / 32767.0f;
}

/** @return Aint equivalent of Afloat */
inline const /* constexpr */ Aint to_Aint(const Afloat &v)
{
    if (v != v) // Abnormal floating point
        return 0;
    else if (v < 0)
        return std::max<int32_t>(static_cast<int32_t>(v * 32768.0f), -32768);
    else
        return std::min<int32_t>(static_cast<int32_t>(v * 32767.0f),  32767);
}

/** Normalized linear magnitude (0.0f - 1.0f) to full-scale decibel */
inline const /* constexpr */ Afloat to_dBFS(const Afloat &v) { return 20.0f * log10(v); }
static const /* constexpr */ Afloat dBFS_limit = to_dBFS(1.0f / 65535.0f);


/**
 * A frame is a snapshot of all sound samples to and/or from all channels at
 * a specific time point. This structure is an expansion of the STL array to
 * support array/frame-wide algorithms and arithmetic.
 */
template< typename T, unsigned char Channels >
struct Aframe
{
    using container_type = std::array< T, Channels >;
    using value_type     = typename container_type::value_type;

    container_type data;

    Aframe() : data() { }
    Aframe(const T* _data, unsigned char _size = Channels) : data()
    {
        std::copy(_data, _data + std::min(_size, Channels), data.begin());
    }

    Aframe(const container_type&  _data) : data(_data) {}
    Aframe(const container_type&& _data) : data(_data) {}

    T       & operator[](unsigned char pos)       { return data[pos]; }
    T const & operator[](unsigned char pos) const { return data[pos]; }

    /* ARITHMETIC */
    void operator+= (const T &v) { for (T& u : data) u += v; }
    void operator-= (const T &v) { for (T& u : data) u -= v; }
    void operator*= (const T &v) { for (T& u : data) u *= v; }
    void operator/= (const T &v) { for (T& u : data) u /= v; }

    template <typename Function>
        void operator()(Function F) {
            for (T &u : data)
                F(u);
        }

    template< unsigned char channels >
        Aframe operator+ (const Aframe<T, channels> &v) const {
            Aframe f;
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                f.data[c] = data[c] + v[c];
            return f;
        }
    template< unsigned char channels >
        Aframe operator- (const Aframe<T, channels> &v) const {
            Aframe f;
            for (unsigned char c = 0; c < std::min<>(Channels,channels); ++c)
                f.data[c] = data[c] - v[c];
            return f;
        }
    template< unsigned char channels >
        Aframe operator* (const Aframe<T, channels> &v) const {
            Aframe f;
            for (unsigned char c = 0; c < std::min<>(Channels,channels); ++c)
                f.data[c] = data[c] * v[c];
            return f;
        }
    template< unsigned char channels >
        Aframe operator/ (const Aframe<T, channels> &v) const {
            Aframe f;
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                f.data[c] = data[c] / v[c];
            return f;
        }

    template< unsigned char channels >
        void operator+= (const Aframe<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] += v[c];
        }

    template< unsigned char channels >
        void operator-= (const Aframe<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] -= v[c];
        }

    template< unsigned char channels >
        void operator*= (const Aframe<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                    data[c] *= v[c];
        }

    template< unsigned char channels >
        void operator/= (const Aframe<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] /= v[c];
        }


    template< unsigned char channels >
        void operator+= (const std::array<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] += v[c];
        }

    template< unsigned char channels >
        void operator-= (const std::array<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] -= v[c];
        }

    template< unsigned char channels >
        void operator*= (const std::array<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] *= v[c];
        }

    template< unsigned char channels >
        void operator/= (const std::array<T, channels> &v) {
            for (unsigned char c = 0; c < std::min<>(Channels, channels); ++c)
                data[c] /= v[c];
        }

    /* ALGORITHMS */
    void abs () {
        for (T& u : data) { u = std::abs(u); }
    }

#ifdef MSC_VER
    const T& max() const {
        return *std::max_element(data.cbegin(), data.cend());
    }

    const T& min() const {
        return *std::min_element(data.cbegin(), data.cend());
    }
#endif

    T absmax() const {
        auto v = std::minmax_element(data.cbegin(), data.cend());
        return std::max(std::fabs(*v.first), std::fabs(*v.second));
    }

    T absmin () const {
        auto v = std::minmax_element(data.cbegin(), data.cend());
        return std::min(std::fabs(*v.first), std::fabs(*v.second));
    }
};

// Standard libawe frame types
typedef Aframe<Aint  , 2> Asintf;       /** a stereo Aint frame   */
typedef Aframe<Afloat, 2> Asfloatf;     /** a stereo Afloat frame */


inline const /* constexpr */ Asfloatf to_Asfloatf(const Asintf &i) {
    return Asfloatf::container_type({ to_Afloat(i[0]), to_Afloat(i[1]) });
}

inline const /* constexpr */ Asintf to_Asintf(const Asfloatf &f) {
    return Asintf  ::container_type({ to_Aint  (f[0]), to_Aint  (f[1]) });
}

struct ArenderConfig
{
    unsigned long targetSampleRate;     /** Target stream sampling rate */
    unsigned long targetFrameCount;     /** Number of frames to render onto target buffer. */
    unsigned long targetFrameOffset;    /** Index on the target buffer to start writing from. */
    enum class renderQuality : uint8_t  /** Render quality option */
    {
        DEFAULT  = 0x0,

        FAST     = 0x1,
        MEDIUM   = 0x2,
        BEST     = 0x3,

        MUTE     = 0xE,
        SKIP     = 0xF
    } quality;

    ArenderConfig(
            unsigned long sample_rate,
            unsigned long frame_count,
            unsigned long frame_offset = 0,
            renderQuality q = renderQuality::DEFAULT
            ) :
        targetSampleRate(sample_rate),
        targetFrameCount(frame_count),
        targetFrameOffset(frame_offset),
        quality(q)
        { }

};

/** Interpolation function
 *
 * An interpolation filter is used to upsample an audio buffer.
 *
 * 4-point, 4th-order optimal 4x z-form interpolation function
 * by Olli Niemitalo, link: http://yehar.com/blog/?p=197
 */
template<
    class T,
    class = typename std::enable_if< std::is_floating_point<T>::value >::type
    > // Define for floating point types only
T interpolate_4p4o_4x_zform(
    T const &x,
    T const &y0, // y[-1]
    T const &y1, // y[ 0] <- x is between
    T const &y2, // y[ 1] <- these values
    T const &y3  // y[ 2]
) {
    T const z = x - 1.0 / 2.0;

    T const e1 = y2 + y1, o1 = y2 - y1;
    T const e2 = y3 + y0, o2 = y3 - y0;

    T const c0 = e1 * 0.46567255120778489 + e2 * 0.03432729708429672;
    T const c1 = o1 * 0.53743830753560162 + o2 * 0.15429462557307461;
    T const c2 = e1 *-0.25194210134021744 + e2 * 0.25194744935939062;
    T const c3 = o1 *-0.46896069955075126 + o2 * 0.15578800670302476;
    T const c4 = e1 * 0.00986988334359864 + e2 *-0.00989340017126506;

    return (((c4*z+c3)*z+c2)*z+c1)*z+c0;
}


} // namespace awe

#endif // AWE_DEFINE
