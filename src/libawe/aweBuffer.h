//  aweBuffer.h :: Data container template definition
//  Copyright 2012 - 2014 Keigen Shu

#ifndef AWE_BUFFER_H
#define AWE_BUFFER_H

#include "aweDefine.h"

#include <cassert>
#include <type_traits>

namespace awe {

/**
 * Audio data container class with interweaving channels.
 *
 * This class is used to contain audio data.
 */
template< typename T >
class Abuffer
{
private:
    unsigned char  channels; //! Number of interwoven audio streams in this buffer
    std::vector<T> pcm_data; //! The vector object holding the audio buffer

public:
    ////    Derive everything from std::vector    //////////////////////
    using container_type    = std::vector<T>;

    using value_type        = typename container_type::value_type;
    using size_type         = typename container_type::size_type;
    using reference         = typename container_type::reference;
    using const_reference   = typename container_type::const_reference;
    using pointer           = typename container_type::pointer;
    using const_pointer     = typename container_type::const_pointer;
    using iterator          = typename container_type::iterator;
    using const_iterator    = typename container_type::const_iterator;

    inline       container_type&  vector()       { return pcm_data; }
    inline const container_type& cvector() const { return pcm_data; }

    inline       reference at(size_type pos)       { return pcm_data.at(pos); }
    inline const_reference at(size_type pos) const { return pcm_data.at(pos); }

    inline       pointer  data()       { return pcm_data.data(); }
    inline const_pointer cdata() const { return pcm_data.data(); }

    inline       iterator  begin()       { return pcm_data. begin(); }
    inline const_iterator  begin() const { return pcm_data. begin(); }
    inline const_iterator cbegin() const { return pcm_data.cbegin(); }
    inline       iterator  end()       { return pcm_data. end(); }
    inline const_iterator  end() const { return pcm_data. end(); }
    inline const_iterator cend() const { return pcm_data.cend(); }

    inline       iterator  rbegin()       { return pcm_data. rbegin(); }
    inline const_iterator  rbegin() const { return pcm_data. rbegin(); }
    inline const_iterator crbegin() const { return pcm_data.crbegin(); }
    inline       iterator  rend()       { return pcm_data. rend(); }
    inline const_iterator  rend() const { return pcm_data. rend(); }
    inline const_iterator crend() const { return pcm_data.crend(); }

    inline void push_back(const T&  value) { pcm_data.push_back(value); }
    inline void push_back(      T&& value) { pcm_data.push_back(value); }

    inline void swap(Abuffer& other) { pcm_data.swap(other.vector()); }

    /**
     * New buffer constructor
     * @param[in] _channels number of channels on buffer
     * @param[in] _frames   number of frames to reserve
     * @param[in] _init     initialze samples in container or leave it empty?
     */
    Abuffer(unsigned char _channels, size_type _frames, bool _init = true)
        : channels(_channels)
        , pcm_data()
    {
        assert(channels != 0 && "Cannot create awe::Abuffer with no channels");

        if (_init == true) {
            pcm_data.reserve(channels * _frames);
            pcm_data.resize (channels * _frames);
        } else {
            pcm_data.reserve(channels * _frames);
        }
    }

    /**
     * Buffer from container constructor
     */
    template< class SourceIterator >
    Abuffer(unsigned char _channels, SourceIterator &_begin, SourceIterator &_end)
        : channels(_channels)
        , pcm_data(_begin, _end)
    {
        assert(channels != 0 && "Cannot create awe::Abuffer with 0 channels");
        assert(pcm_data.size() % channels == 0);
    }

    /**
     * Buffer copy constructor
     * @param[in] _other buffer to copy from
     */
    Abuffer(const Abuffer &_other)
        : Abuffer(_other.getChannelCount(), _other.cbegin(), _other.cend())
    { }

    /**
     * Buffer destructor
     */
    ~Abuffer() { pcm_data.clear(); }

    /**
     * Frees the memory allocated by the buffer.
     * @return Number of samples on the old buffer.
     */
    size_type clear()
    {
        size_type samples = pcm_data.size();
        pcm_data.clear  ( );
        pcm_data.reserve(0);
        return samples;
    }

    /**
     * Flushes the entire buffer to zero.
     *
     * The size of new buffer is rounded down to the smallest frame.
     *
     * @return Number of samples on the old buffer.
     */
    size_type reset()
    {
        size_type samples = pcm_data.size();
        size_type new_size = (samples / channels) * channels;
        pcm_data.clear  ( );
        pcm_data.reserve(new_size);
        pcm_data.resize (new_size);
        return samples;
    }

    inline   unsigned char getChannelCount() const { return channels;                    } /** @return number of channels in the buffer             */
    inline       size_type getBufferSize  () const { return pcm_data.size() * sizeof(T); } /** @return size of the entire buffer       (in bytes)   */
    inline       size_type getSampleCount () const { return pcm_data.size();             } /** @return number of samples in the buffer (in samples) */
    inline const size_type getSampleSize  () const { return sizeof(T);                   } /** @return size of a single sample         (in bytes)   */
    inline       size_type getFrameCount  () const { return pcm_data.size() / channels;  } /** @return number of frames in the buffer  (in frames)  */
    inline       size_type getFrameSize   () const { return sizeof(T) * channels;        } /** @return size of a single frame          (in bytes)   */

    inline       pointer  getFrame (size_type pos = 0)       /** @return          pointer to frame in buffer at frame offset index or nullptr if offset is out-of-bounds */
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }

    inline const_pointer cgetFrame (size_type pos = 0) const /** @return constant pointer to frame in buffer at frame offset index or nullptr if offset is out-of-bounds */
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }

    /** @return copy of frame in buffer at frame offset index or empty frame if offset is out-of-bounds */
    template< unsigned char Channels >
    Aframe<value_type, Channels> getFrame (size_type pos = 0) const
    { return (pcm_data.size() > pos * channels)
                ? Aframe<value_type, Channels>(pcm_data.data() + (pos * channels), channels) // initializes up to `channels` elements
                : Aframe<value_type, Channels>();
    }

    inline value_type getSample (size_type pos = 0) const /** @return copy of sample in buffer at sample offset index or an uninitialized value if offset is out-of-bounds */
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(); }

    inline value_type get0Sample(size_type pos = 0) const /** @return copy of sample in buffer at sample offset index or zero if offset is out-of-bounds */
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(0); }

    inline value_type getiSample(double    pos = 0) const /** @return interpolated sample from buffer at sample offset or zero if offset is out-of-bounds */
    {
        size_type x = std::floor(pos);
        double    t = pos - x;

        if (std::is_floating_point<value_type>::value)
        {
            double y0 = get0Sample(x - 1 * channels);
            double y1 = get0Sample(x);
            double y2 = get0Sample(x + 1 * channels);
            double y3 = get0Sample(x + 2 * channels);

            return interpolate_4p4o_4x_zform(t, y0, y1, y2, y3);
        }
        else
        {
            double y0 = to_Afloat(get0Sample(x - 1 * channels));
            double y1 = to_Afloat(get0Sample(x));
            double y2 = to_Afloat(get0Sample(x + 1 * channels));
            double y3 = to_Afloat(get0Sample(x + 2 * channels));

            return to_Aint(interpolate_4p4o_4x_zform(t, y0, y1, y2, y3));
        }
    }

    inline void setSample (size_type pos, value_type value)
    {
        pcm_data[pos] = value;
    }

    inline void setFrame  (size_type pos, value_type value)
    {
        for(size_type i=0; i<channels; ++i)
            pcm_data[pos*channels  ] = value;
    }

    inline void setFrame  (size_type pos, const_pointer value)
    {
        for(size_type i=0; i<channels; ++i)
            pcm_data[pos*channels+i] = value[i];
    }

    template< unsigned char Channels >
    void setFrame  (size_type pos, const Aframe<value_type, Channels> value)
    {
        for(size_type i=0; i<std::min(Channels,channels); ++i)
            pcm_data[pos*channels+i] = value[i];
    }
};

typedef Abuffer<Aint  > AiBuffer;
typedef Abuffer<Afloat> AfBuffer;

} // namespace awe

#endif // AWE_BUFFER_H
