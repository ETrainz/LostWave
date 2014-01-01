//  aweBuffer.h :: Data container template definition
//  Copyright 2012 - 2013 Keigen Shu

#ifndef AWE_BUFFER_H
#define AWE_BUFFER_H

#include <cassert>
#include <cmath>

#include "aweDefine.h"

namespace awe {

/** Audio data container class.
 * This class is used to contain PCM audio data.
 */
template <typename T> class Abuffer
{
private:
    std::vector<T>      pcm_data;
    const unsigned char channels;

public:
    /* Derive useful std::vector types and functions */
    typedef typename std::vector<T>                     container_type;
    typedef typename container_type::value_type         value_type;
    typedef typename container_type::size_type          size_type;
    typedef typename container_type::reference          reference;
    typedef typename container_type::const_reference    const_reference;
    typedef typename container_type::pointer            pointer;
    typedef typename container_type::const_pointer      const_pointer;
    typedef typename container_type::iterator           iterator;
    typedef typename container_type::const_iterator     const_iterator;

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
     * @param _channels number of channels on buffer
     * @param _frames   number of frames to reserve
     * @param _init     initialze samples in container or leave it empty?
     */
    Abuffer(unsigned char _channels, size_type _frames, bool init = true) :
        pcm_data(),
        channels(_channels)
        {
            assert(channels != 0);

            if (init == true) {
                pcm_data.reserve(_channels * _frames);
                pcm_data.resize (_channels * _frames);
            } else {
                pcm_data.reserve(_channels * _frames);
            }
        }

    Abuffer(unsigned char _channels, container_type const &_container) :
        pcm_data(_container),
        channels(_channels)
        {
            assert(_channels != 0);
            assert(_container.size() % _channels == 0);
        }

    Abuffer(const Abuffer &buffer) :
        pcm_data(buffer.cvector()),
        channels(buffer.getChannelCount())
        { }

    ~Abuffer() { pcm_data.clear(); }


    inline void reset(bool init = true)
    {
        if (init == true) {
            size_type samples = (pcm_data.size() / channels) * channels;
            pcm_data.clear();
            pcm_data.resize(samples);
        } else {
            pcm_data.clear();
        }
    }

    inline   unsigned char getChannelCount() const { return channels;                    } /** @return number of channels in the buffer             */
    inline       size_type getBufferSize  () const { return pcm_data.size() * sizeof(T); } /** @return size of the entire buffer       (in bytes)   */
    inline       size_type getSampleCount () const { return pcm_data.size();             } /** @return number of samples in the buffer (in samples) */
    inline const size_type getSampleSize  () const { return sizeof(T);                   } /** @return size of a single sample         (in bytes)   */
    inline       size_type getFrameCount  () const { return pcm_data.size() / channels;  } /** @return number of frames in the buffer  (in frames)  */
    inline       size_type getFrameSize   () const { return sizeof(T) * channels;        } /** @return size of a single frame          (in bytes)   */

    inline value_type    getSample (size_type pos = 0) const /** @return copy of sample in buffer at sample offset index or an uninitialized value if offset is out-of-bounds */
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(); }
    inline       pointer getFrame  (size_type pos = 0)       /** @return          pointer to frame in buffer at frame offset index or nullptr if offset is out-of-bounds */
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }
    inline const_pointer getFrame  (size_type pos = 0) const /** @return constant pointer to frame in buffer at frame offset index or nullptr if offset is out-of-bounds */
    { return (pcm_data.size() > pos * channels) ? pcm_data.data() + (pos * channels) : nullptr; }

    inline value_type    get0Sample(size_type pos = 0) const /** @return copy of sample in buffer at sample offset index or zero if offset is out-of-bounds */
    { return (pcm_data.size() > pos) ? pcm_data[pos] : value_type(0); }
    inline value_type    getiSample(double    pos = 0) const
    {
        size_type x = std::floor(pos);
        double    t = pos - x;

        double y0 = get0Sample(pos - 1*channels);
        double y1 = get0Sample(pos);
        double y2 = get0Sample(pos + 1*channels);
        double y3 = get0Sample(pos + 2*channels);

        return interpolate_4p4o_4x_zform(t, y0, y1, y2, y3);
    }

    void setSample (size_type pos, value_type value)
    { pcm_data[pos] = value; }

    void setFrame  (size_type pos, value_type value)
    {
        for(size_type i=0; i<channels; ++i)
            pcm_data[pos*channels  ] = value;
    }

    void setFrame  (size_type pos, const_pointer value)
    {
        for(size_type i=0; i<channels; ++i)
            pcm_data[pos*channels+i] = value[i];
    }
};

typedef Abuffer<Aint>   AiBuffer;
typedef Abuffer<Afloat> AfBuffer;

} // namespace awe

#endif // AWE_BUFFER_H
