//  aweBuffer.h :: Data container template definition
//  Copyright 2012 - 2014 Chu Chin Kuan <keigen.shu@gmail.com>

#ifndef AWE_BUFFER_H
#define AWE_BUFFER_H

#include "aweDefine.h"

#include <cassert>
#include <type_traits>

namespace awe {

/*! Audio data container class with interweaving channels.
 *  This class is used to contain audio data.
 */
template< typename T >
class Abuffer
{
private:
    unsigned char  channels; //!< Number of interwoven audio streams in this buffer.
    std::vector<T> pcm_data; //!< The vector object holding the audio buffer.

public:
    //! \name std::vector member type alias
    //! \{

    //! Type alias for the container.
    using container_type    = std::vector<T>;

    //! Type alias for the value contained in the container.
    using value_type        = typename container_type::value_type;
    //! Type alias for the size of the container.
    using size_type         = typename container_type::size_type;
    //! Type alias for the reference to the value contained in the container.
    using reference         = typename container_type::reference;
    //! Type alias for the constant reference to the value contained in the container.
    using const_reference   = typename container_type::const_reference;
    //! Type alias for the pointer to the value contained in the container.
    using pointer           = typename container_type::pointer;
    //! Type alias for the constant pointer to the value contained in the container.
    using const_pointer     = typename container_type::const_pointer;
    //! Type alias for the value iterator for the container.
    using iterator          = typename container_type::iterator;
    //! Type alias for the constant value iterator in the container.
    using const_iterator    = typename container_type::const_iterator;
    //! \}

    //! \name std::vector member functions
    //! \{
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

    /*! Swaps the internal buffer of this buffer with that in another buffer.
     *  \warning This function does not swap the channel count of the buffer.
     *  \param other the buffer to swap with
     */
    inline void swap(Abuffer& other) { pcm_data.swap(other.vector()); }
    //! \}

    /*! New buffer constructor.
     *  \param[in] _channels number of channels on buffer
     *  \param[in] _frames   number of frames to reserve
     *  \param[in] _init     initialize samples in container or leave it empty?
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

    /*! Constructs a buffer from a C++ iterable range.
     *  \tparam InputIterator    type of the input iterator
     *  \param[in] _channels     number of channels on buffer
     *  \param[in] _begin, _end  the range of elements to copy from
     */
    template< class InputIterator >
    Abuffer(unsigned char _channels, InputIterator &_begin, InputIterator &_end)
        : channels(_channels)
        , pcm_data(_begin, _end)
    {
        assert(channels != 0 && "Cannot create awe::Abuffer with 0 channels");
        assert(pcm_data.size() % channels == 0);
    }

    /*! Buffer copy constructor.
     *  \param[in] _other buffer to copy from
     */
    Abuffer(const Abuffer &_other)
        : Abuffer(_other.getChannelCount(), _other.cbegin(), _other.cend())
    { }

    /*! Buffer destructor.
     */
    ~Abuffer() { pcm_data.clear(); }

    /*! Frees the memory allocated by the buffer.
     *  \return number of samples on the old buffer.
     */
    size_type clear()
    {
        size_type samples = pcm_data.size();
        pcm_data.clear  ( );
        pcm_data.reserve(0);
        return samples;
    }

    /*! Flushes the entire buffer to zero.
     *  The size of new buffer is rounded down to the nearest lower frame.
     *  \return number of samples on the old buffer.
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

    /*! \return number of channels in the buffer             */
    inline  unsigned char      getChannelCount() const { return channels;                    }
    /*! \return size of the entire buffer       (in bytes)   */
    inline           size_type getBufferSize  () const { return pcm_data.size() * sizeof(T); }
    /*! \return number of samples in the buffer (in samples) */
    inline           size_type getSampleCount () const { return pcm_data.size();             }
    /*! \return size of a single sample         (in bytes)   */
    inline constexpr size_type getSampleSize  () const { return sizeof(T);                   }
    /*! \return number of frames in the buffer  (in frames)  */
    inline           size_type getFrameCount  () const { return pcm_data.size() / channels;  }
    /*! \return size of a single frame          (in bytes)   */
    inline           size_type getFrameSize   () const { return sizeof(T) * channels;        }

    /*! Returns a pointer to the frame in the buffer.
     *  \param pos frame offset index
     *  \return pointer to frame in buffer or nullptr if offset is out-of-bounds.
     */
    inline       pointer  getFrame (size_type pos = 0)
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }

    /*! Returns a constant pointer to the frame in the buffer.
     *  \param pos frame offset index
     *  \return constant pointer to frame in buffer or nullptr if offset is out-of-bounds.
     */
    inline const_pointer cgetFrame (size_type pos = 0) const
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }


    /*! Returns a copy of the frame in the buffer.
     *  \tparam Channels number of channels on the output frame object.
     *  \param pos frame offset index
     *  \return copy of frame in buffer or empty frame if offset is out-of-bounds.
     */
    template< unsigned char Channels >
    Aframe<value_type, Channels> getFrame (size_type pos = 0) const
    { return (pcm_data.size() > pos * channels)
                ? Aframe<value_type, Channels>(pcm_data.data() + (pos * channels), channels) // initializes up to `channels` elements
                : Aframe<value_type, Channels>();
    }

    /*! Returns a copy of the sample in the buffer.
     *  \param pos sample offset index
     *  \return copy of sample in buffer or an uninitialized/compiler default value if offset is out-of-bounds.
     *  \warning Possible undefined behaviour if the supplied offset is out-of-bounds.
     *  \see get0Sample A variant that returns zero if the offset is out-of-bounds.
     */
    inline value_type getSample (size_type pos = 0) const
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(); }

    /*! Returns a copy of the sample in the buffer.
     *  \param pos sample offset index
     *  \return copy of sample in buffer or zero if offset is out-of-bounds.
     */
    inline value_type get0Sample(size_type pos = 0) const
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(0); }

    /*! Returns an interpolated sample from the buffer.
     *  \param pos sample offset
     *  \return interpolated sample from buffer.
     *  \see interpolate_4p4o_4x_zform Implementation of the interpolation function.
     */
    inline value_type getiSample(double    pos = 0) const
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

//! \name Standard audio buffer types
//! \{
typedef Abuffer<Aint  > AiBuffer; //!< Integer audio buffer
typedef Abuffer<Afloat> AfBuffer; //!< Floating-point audio buffer
//! \}

} // namespace awe

#endif // AWE_BUFFER_H
