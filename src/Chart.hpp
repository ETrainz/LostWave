//  Chart.hpp :: Chart class object declaration
//  Copyright 2011 - 2014 Keigen Shu

#ifndef CHART_H
#define CHART_H

#include <string>
#include "__zzCore.hpp"
#include "Measure.hpp"
#include "AudioManager.hpp"

class Chart
{
protected:
    std::string         name;       //! Name of this chart
    std::string         charter;    //! Name of the person who made this chart
    unsigned int        level;      //! Difficulty rating number of this chart
    unsigned int        events;     //! Number of event objects
    unsigned int        notes;      //! Number of note objects
    unsigned int        duration;   //! Length of chart in seconds
    double              tempo;      //! Starting tempo of this chart in BPM

    clan::PixelBuffer   cover;      //! The cover art pixel buffer for this chart
    Sequence            sequence;   //! The event sequence object
    SampleMap           sample_map; //! The ID to Sample map for this chart

    bool       cover_loaded;
    bool    sequence_loaded;
    bool     samples_loaded;

public:
    Chart() :
        cover           (),
        cover_loaded    (false),
        sequence_loaded (false),
        samples_loaded  (false)
    {}

    virtual ~Chart() { this->clear(); }

    virtual void load_art     () = 0;
    virtual void load_chart   () = 0;
    virtual void load_samples () = 0;

    inline  void sort_sequence() { for (Measure* m : sequence) m->sort_lists(); }
    inline  void load()
    {
        std::thread art(
            [this]() {
                this->load_art();
                this->load_samples();
            }
        );

        std::thread chart(
            [this]() {
                this->load_chart();
                this->sort_sequence();
            }
        );

        art.join();
        chart.join();
    }

    void clear();

    /**
     * Calculates the distance (in ticks) between two time points in
     * this chart.
     * @note This function is slow as it iterates over every measure
     *       from point a to point b.
     */
    long compare_ticks(const TTime &a, const TTime &b) const;
    /**
     * Calculates the time point (in seconds) of the given tick
     * time in this chart.
     *
     * @note This function is requires that all parameter events are
     *       aligned properly.
     * @note This function is slow as it iterates over every measure
     *       from the beginning to point t.
     */
    double translate(const TTime &t) const;


    inline std::string  getName     () const { return name; }
    inline std::string  getCharter  () const { return charter; }
    inline unsigned int getLevel    () const { return level; }
    inline unsigned int getEvents   () const { return events; }
    inline unsigned int getNotes    () const { return notes; }

    inline unsigned int getDuration () const { return duration; }
    inline double       getTempo    () const { return tempo; }


    inline       clan::PixelBuffer &  getCoverArt () { return cover; }
    inline const clan::PixelBuffer & cgetCoverArt () { return cover; }
    inline void setCoverArt (clan::PixelBuffer const &_cover) { cover = _cover.copy(); }
    inline void setCoverArt ()                                { cover = clan::PixelBuffer(); }

    inline const Measure   * cgetMeasure   (size_t index) const { return (sequence.size() > index) ? sequence[index] : nullptr; }
    inline       Measure   *  getMeasure   (size_t index)       { return (sequence.size() > index) ? sequence[index] : nullptr; }
    inline size_t             getMeasures  () const { return  sequence.size(); }

    inline const Sequence  & cgetSequence  () const { return sequence; }
    inline       Sequence  &  getSequence  ()       { return sequence; }

    inline const SampleMap & cgetSampleMap () const { return sample_map; }
    inline       SampleMap &  getSampleMap ()       { return sample_map; }
};

#endif
