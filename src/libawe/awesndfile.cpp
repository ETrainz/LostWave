//  awesndfile.cpp :: Audio file reader via libsndfile
//  Copyright 2012 - 2013 Keigen Shu

#include <exception>
#include "awesndfile.h"
#include "aweSample.h"


namespace awe {

/* read data from SNDFILE into sample */
void read_sndfile(Asample* sample, SNDFILE* sndf, SF_INFO* info)
{
    // Second buffer needed for overclip bug workaround
    AiBuffer* bufi = new AiBuffer(info->channels, info->frames, false);
    AfBuffer* buff = new AfBuffer(info->channels, info->frames, true );

    // Read as float
    sf_readf_float(sndf, buff->data(), info->frames);

    // Find peak sample value in file.
    float peakValue;

    if (sf_command(sndf, SFC_GET_SIGNAL_MAX, &peakValue, sizeof(peakValue)) == SF_FALSE)
    {
        // Peak value not provided by file, calculate ourself.
        peakValue = 1.0f;

        for (const Afloat &x : *buff) {
            if (std::fabs(x) > peakValue)
                peakValue = std::fabs(x);
        }
    }

    // Fix and copy buffer
    for (const Afloat &x : *buff) {
        bufi->push_back(to_Aint(x / peakValue));
    }

    delete buff;

    // Save buffer into sample, clean up and return
    sample->setSource(bufi, peakValue, info->samplerate);

    sf_close(sndf);

    return;
}


/* Read from memory -- libsndfile virtual IO functions */

sf_count_t awe_sf_vmio_get_filelen(void *user_data)
{
    awe_sf_vmio_data* io = (awe_sf_vmio_data*)user_data;
    return io->size;
}

sf_count_t awe_sf_vmio_tell(void *user_data)
{
    awe_sf_vmio_data* io = (awe_sf_vmio_data*)user_data;
    return io->curr;
}

sf_count_t awe_sf_vmio_seek(sf_count_t offset, int whence, void *user_data)
{
    awe_sf_vmio_data* io = (awe_sf_vmio_data*)user_data;

    switch (whence)
    {
        case SEEK_SET: io->curr =        0 + offset; break;
        case SEEK_CUR: io->curr = io->curr + offset; break;
        case SEEK_END: io->curr = io->size; break;
        default: throw std::exception(); break;
    }

    return io->curr;
}

sf_count_t awe_sf_vmio_read(void *ptr, sf_count_t count, void *user_data)
{
    awe_sf_vmio_data* io = (awe_sf_vmio_data*)user_data;
    sf_count_t realcount = 0;
    char* sptr = (char*)ptr;

    for (sf_count_t i=0; i<count; i++) {
        if (io->curr < io->size) {
            sptr[i] = io->mptr[io->curr];
            io->curr++;
            realcount++;
        }
    }

    return realcount;
}

sf_count_t awe_sf_vmio_write(const void *ptr, sf_count_t count, void *user_data)
{
    awe_sf_vmio_data* io = (awe_sf_vmio_data*)user_data;
    sf_count_t realcount = 0;
    char* sptr = (char*)ptr;

    for (sf_count_t i=0; i<count; i++) {
        if (io->curr < io->size) {
            io->mptr[io->curr] = sptr[i];
            io->curr++;
            realcount++;
        }
    }

    return realcount;
}

// Asample constructors
Asample::Asample(
        const std::string & file,
        const Aloop::Mode &_loop
        ) :
    Asource(),
    mSource(nullptr),
    mSourcePeak(1.0),
    mSampleRate(0),
    mSampleName(file),
    mMixer(1.0, 0.0),
    mLoop(0, 0, 0, _loop)
{
    SF_INFO* info = new SF_INFO;
    SNDFILE* sndf;

    sndf = sf_open(file.c_str(), SFM_READ, info);

    if (sf_error(sndf) != SF_ERR_NO_ERROR) {
        fprintf(stderr,"libsndfile [error] %s: %s.\n", file.c_str(), sf_strerror(sndf));
        sf_close(sndf);
        return;
    }

    if (info->channels > 2) {
        fprintf(stderr,"libawe [error] %s: libawe currently only supports mono and stereo. \n", file.c_str());
        sf_close(sndf);
        return;
    }

    read_sndfile(this, sndf, info);

    return;
}

Asample::Asample(
        char              * mptr,
        const size_t      & size,
        const std::string &_name,
        const Aloop::Mode &_loop
        ) :
    Asource(),
    mSource(nullptr),
    mSourcePeak(1.0),
    mSampleRate(0),
    mSampleName(_name),
    mMixer(1.0, 0.0),
    mLoop(0, 0, 0, _loop)
{
    SF_INFO* info = new SF_INFO;
    SNDFILE* sndf;
    awe_sf_vmio_data vmiod = { 0, static_cast<sf_count_t>(size), mptr };

    sndf = sf_open_virtual(&awe_sf_vmio, SFM_READ, info, (void*)&vmiod);

    if (sf_error(sndf) != SF_ERR_NO_ERROR) {
        fprintf(stderr,"libsndfile [error] %p: %s.\n", mptr, sf_strerror(sndf));
        sf_close(sndf);
        return;
    }

    if (info->channels > 2) {
        fprintf(stderr,"libawe [error] %p: libawe currently only supports mono and stereo. \n", mptr);
        sf_close(sndf);
        return;
    }

    read_sndfile(this, sndf, info);

    return;
}

}
