//  aweDefine.h :: Essential declarations and definitions
//  Copyright 2012 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_DEFINE
#define AWE_DEFINE

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#include <algorithm>
#include <array>
#include <queue>

//! \brief The libawe namespace, where everything used by libawe resides in.
namespace awe {

//!@name Standard audio data types
//!@{
using Aint   = int16_t; //!< 16-bit integer data type
using Afloat = float;   //!< Architecture-specific floating point data type
//!@}

//!@name Standard audio queue buffers
//!@{
typedef std::queue<Aint  > AiFIFOBuffer; //!< Integer audio data queue
typedef std::queue<Afloat> AfFIFOBuffer; //!< Floating-point audio data queue
//!@}

#define IO_BUFFER_SIZE  16384   //!< Default file IO buffer size

//!@name Standard data type converters
//!@{

//! @brief 16-bit bi-polar integer to normalized floating point value casting function.
//! @return Afloat equivalent of Aint
inline Afloat to_Afloat(const Aint &v)
{
    if (v == 0)
        return 0;
    else if (v < 0)
        return float(v) / 32768.0f;
    else
        return float(v) / 32767.0f;
}

//! @brief Normalized floating point to 16-bit bi-polar integer value casting function.
//! @return Aint equivalent of Afloat
inline Aint to_Aint(const Afloat &v)
{
    if (v != v) // Abnormal floating point
        return 0;
    else if (v < 0)
        return std::max<Aint>(static_cast<int32_t>(v * 32768.0f), -32768);
    else
        return std::min<Aint>(static_cast<int32_t>(v * 32767.0f),  32767);
}
//!@}

//! Converts a normalized linear magnitude value (0.0f - 1.0f) to a full-scale decibel magnitude value
inline Afloat to_dBFS(const Afloat &v) { return 20.0f * log10(v); }

//! Smallest unit of representation for a 16-bit integer on a normalized floating point.
static constexpr Afloat Afloat_limit = 1.0f / 65535.0f;

//! Full scale decibel limit (~96 dB).
static const Afloat dBFS_limit = to_dBFS(Afloat_limit);


static const Afloat Afloat_96dB = 1.0 / pow(10.0, 4.8);

/*! Structure holding a single audio frame.
 *  A frame is a snapshot of all sound samples on all channels at a specific
 *  time point. This structure is designed as an expansion of the STL array to
 *  support array/frame-wide algorithms and arithmetic.
 *
 *  @tparam T type of data that this structure will hold.
 *  @tparam Channels number of channels of data within this single frame.
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

//!@name Standard libawe frame types
//!@{
typedef Aframe<Aint  , 2> Asintf;       //!< a stereo Aint frame
typedef Aframe<Afloat, 2> Asfloatf;     //!< a stereo Afloat frame
//!@}

//!@name Standard data type converters
//!@{
inline const /* constexpr */ Asfloatf to_Asfloatf(const Asintf &i) {
    return Asfloatf::container_type({ to_Afloat(i[0]), to_Afloat(i[1]) });
}

inline const /* constexpr */ Asintf to_Asintf(const Asfloatf &f) {
    return Asintf  ::container_type({ to_Aint  (f[0]), to_Aint  (f[1]) });
}
//!@}

/*! General purpose audio rendering configuration structure passed to modules.
 */
struct ArenderConfig
{
    /** Render quality option */
    enum class Quality : uint8_t
    {
        /** Default rendering configuration */
        DEFAULT  = 0x0,
        /** Fastest rendering option */
        FAST     = 0x1,
        /** Balanced rendering option */
        MEDIUM   = 0x2,
        /** Best render quality */
        BEST     = 0x3,
        /** Update renderer without doing anything to the target buffer. */
        MUTE     = 0xE,
        /** Return immediately without doing anything. */
        SKIP     = 0xF
    };

    /** Target stream sampling rate. */
    unsigned long targetSampleRate;

    /** Number of frames to render onto target buffer. */
    unsigned long targetFrameCount;

    /** Index on the target buffer to start writing from. */
    unsigned long targetFrameOffset;

    /** Rendering quality. */
    Quality quality;

    /** Default constructor. */
    ArenderConfig(
        unsigned long sample_rate,
        unsigned long frame_count,
        unsigned long frame_offset = 0,
        Quality q = Quality::DEFAULT
    )   : targetSampleRate(sample_rate)
        , targetFrameCount(frame_count)
        , targetFrameOffset(frame_offset)
        , quality(q)
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
