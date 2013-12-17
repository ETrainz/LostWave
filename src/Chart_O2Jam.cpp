#include "Chart_O2Jam.hpp"
#include "Music.hpp"

namespace O2Jam {

//!~ OJN Music Parser
Music* openOJN (const std::string& path)
{
    clan::File file;
    if (file.open(path,
                clan::File::OpenMode::open_existing,
                clan::File::AccessFlags::access_read
                ) == false)
        return nullptr;

    char* buffer = new char[300];
    int read = file.read(buffer, 300);
    if (read != 300) {
        delete[] buffer;
        return nullptr;
    }

    OJN_Header *pHead = (OJN_Header*)((void*)buffer);

    // parse header
    Music* music = new Music;
    music->path     = path;
    music->title    = pHead->Title;
    music->artist   = pHead->Artist;
    music->genre    = pHead->oldGenre;

    switch(pHead->newGenreCode) {
        case 0 : music->genre = "Ballad"; break;
        case 1 : music->genre = "Rock"; break;
        case 2 : music->genre = "Dance"; break;
        case 3 : music->genre = "Techno"; break;
        case 4 : music->genre = "Hip-hop"; break;
        case 5 : music->genre = "Soul/R&B"; break;
        case 6 : music->genre = "Jazz"; break;
        case 7 : music->genre = "Funk"; break;
        case 8 : music->genre = "Traditional"; break;
        case 9 : music->genre = "Classical"; break;
        case 10: music->genre = "Others"; break;
    };

    // initialise charts
    O2JamChart *ex = new O2JamChart(path, *pHead, 0);
    O2JamChart *nx = new O2JamChart(path, *pHead, 1);
    O2JamChart *hx = new O2JamChart(path, *pHead, 2);

    // sync
    music->charts[0] = (Chart*)ex;
    music->charts[1] = (Chart*)nx;
    music->charts[2] = (Chart*)hx;

    delete[] buffer;
    return music;
} // end openOJN

O2JamChart::O2JamChart(const std::string &path, const OJN_Header &header, uint8_t index) :
    ojn_path   (path),
    ojn_header (header),
    chart_index(index)
{
    if (index > 2)
        throw std::invalid_argument("Invalid chart index.");

    name        = (index == 2 ? "HX" : (index == 1 ? "NX" : "EX"));
    charter     = header.Charter;
    level       = header.Level[index];
    events      = header.numEvents[index];
    notes       = header.numNotes[index];
    duration    = header.Duration[index];
    tempo       = header.Tempo;

    ojm_path = clan::PathHelp::get_basepath(path) + header.OJMFile;
}



void O2JamChart::load_art ()
{
    if (ojn_header.newCoverArtSize == 0)
        return;

    clan::File file;
    if (file.open(ojn_path,
                clan::File::OpenMode::open_existing,
                clan::File::AccessFlags::access_read
                ) == false)
        return;

    file.seek(ojn_header.DataOffset[3], clan::File::seek_set);

    uint8_t* buffer = new uint8_t[ojn_header.newCoverArtSize];

    int read = file.read(buffer, ojn_header.newCoverArtSize);

    if (read == static_cast<int>(ojn_header.newCoverArtSize)) {
        try
        {
            clan::DataBuffer        dbuff ( reinterpret_cast<const void*>(buffer), read );
            clan::IODevice_Memory   memio ( dbuff );
            clan::PixelBuffer       cover ( memio, "jpg", false );

            this->setCoverArt(cover);
            this->cover_loaded = true;
        } catch (clan::Exception &e) {
            clan::Console::write_line("Failed to load .jpg cover art for %1.", ojn_header.Title);
            clan::Console::write_line(e.get_message_and_stack_trace());
        }
    }

    delete[] buffer;
}

void O2JamChart::load_chart ()
{
    clear();

    TTime       time;
    Measure*   pMeasure;

    // LONG note lists
    SingleNoteList HNL, RNL;

    // Counters : Params, Notes, SingleNotes, LongNotes, HoldNotes, ReleaseNotes, PlayNotes, AutoNotes.
    size_t cP = 0, cN = 0, cSN = 0, cLN = 0, cHN = 0, cRN = 0, cPN = 0, cAN = 0;


    for (unsigned n = 0; n <= ojn_header.numMeasures[chart_index]; n++)
        sequence.push_back(new Measure(4, 48));

    clan::File file;

    if( file.open(ojn_path, clan::File::open_existing, clan::File::access_read) == false )
        return;

    unsigned long chartSize = file.get_size() - ojn_header.DataOffset[chart_index];

    file.seek(ojn_header.DataOffset[chart_index], clan::File::seek_set);
    char* buffer = new char[chartSize];
    int read = file.read(buffer, chartSize);
    if (read != (int)chartSize)
        delete buffer;

    const char *pPtr = (char*)((void*)buffer);

    // OJN Note Set parse loop
    for (unsigned j = 0; j < ojn_header.numNoteSets[chart_index]; j++)
    {
        OJN_NoteSet_Header *pNoteSet = (OJN_NoteSet_Header*)pPtr;
        pPtr += sizeof(OJN_NoteSet_Header);

        uint32_t iMeasure  = pNoteSet->Measure;
        uint16_t iChannel  = pNoteSet->Channel;
        uint16_t numEvents = pNoteSet->numEvents;

        pMeasure = sequence[iMeasure];

        ENKey nChannel = ENKey::NOTE_AUTO;

        switch (iChannel)
        {
            // Time Signature changes
            case 0:
                pMeasure->setTimeSignature(*((float*)pPtr));

                pPtr += (4 * numEvents);
                break;
                // Tempo changes
            case 1:
                for (unsigned k = 0; k < numEvents; k++)
                {
                    if (*((float*)pPtr) != 0.0f)
                    {
                        uint16_t Tick = k * 192 / numEvents;
                        time = TTime(Tick % 48, Tick / 48, iMeasure);

                        ParamEvent* pParamEvent = new ParamEvent(time, EParam::EP_C_TEMPO, *((float*)pPtr));
                        cP++;

                        pMeasure->addParamEvent(pParamEvent);
                    }

                    pPtr += 4;
                }
                break;

                // Music Notes
            default:
                switch (iChannel) {
                    case 2: nChannel = ENKey::NOTE_P1_1; break;
                    case 3: nChannel = ENKey::NOTE_P1_2; break;
                    case 4: nChannel = ENKey::NOTE_P1_3; break;
                    case 5: nChannel = ENKey::NOTE_P1_4; break;
                    case 6: nChannel = ENKey::NOTE_P1_5; break;
                    case 7: nChannel = ENKey::NOTE_P1_6; break;
                    case 8: nChannel = ENKey::NOTE_P1_7; break;
                }

                for (uint16_t k = 0; k < numEvents; k++)
                {
                    uint16_t Tick = k * 192 / numEvents;

                    time = TTime(Tick % 48, Tick / 48, iMeasure);

                    // read note event
                    OJN_Note *pEvent = (OJN_Note*)pPtr;
                    pPtr += 4;

                    uint16_t SmplID = pEvent->SampleID;
                    uint8_t  VolPan = pEvent->VolPan;
                    uint8_t  Type   = pEvent->NoteType;

                    // skip empty notes; sample IDs always start with 1
                    if (SmplID != 0)
                    {
                        float Vol = (float)((VolPan >> 4) & 0x0F); // higher half
                        float Pan = (float) (VolPan & 0x0F);       //  lower half

                        Vol = (Vol == 0) ? 1.0f : (Vol / 16.0f);
                        Pan = (Pan == 0) ? 0.0f : (Pan -  8.0f) / 8.0f;

                        // Note Type safeguard
                        if (Type >= 8) {
                            printf("[debug] Fixing invalid note type. \n");
                            Type = Type % 8;
                        }

                        // M### mode
                        if (Type >= 4) {
                            SmplID += 1000;
                            Type -= 4;
                        }

                        // set up event
                        cN++;

                        if (nChannel == ENKey::NOTE_AUTO)
                        {
                            nChannel = ENKey_toBG(iChannel - 9);
                            // Silently remove release notes.
                            if (Type == 2) {
                                Note_Single* pNote = new Note_Single(nChannel, time, SmplID, Vol, Pan);
                                pMeasure->addNote(pNote);
                                cAN++, cSN++;
                                printf("[debug] Parsing to Hold Note in Autoplay Channel as Normal Note. \n");
                            } else if (Type == 3) {
                                printf("[debug] Skipping Release Note in Autoplay Channel. \n");
                            } else {
                                Note_Single* pNote = new Note_Single(nChannel, time, SmplID, Vol, Pan);
                                pMeasure->addNote(pNote);
                                cAN++, cSN++;
                            }
                        } else if (Type == 2) { // Hold Notes
                            Note_Single* pNote = new Note_Single(nChannel, time, SmplID, Vol, Pan);
                            HNL.push_back(pNote);
                            cHN++;
                        } else if (Type == 3) { // Release Notes
                            Note_Single* pNote = new Note_Single(nChannel, time, SmplID, Vol, Pan);
                            RNL.push_back(pNote);
                            cRN++;
                        } else {
                            Note_Single* pNote = new Note_Single(nChannel, time, SmplID, Vol, Pan);
                            pMeasure->addNote(pNote);
                            cAN++, cPN++;
                        }

                    } // end-if empty sample
                } // end-for

                break;
        } // end switch

    }

    // Resolve links between hold and release notes.
    NoteList L = zip_to_long_notes(HNL, RNL);
    while(!L.empty())
    {
        auto it = L.begin();
        Note* n = *it;
        sequence[n->getTime().measure]->addNote(n);
        L.erase(it);
        cLN++;
    }

    sort_sequence();
    this->notes = cPN;
    this->sequence_loaded = true;

    delete[] buffer;

}

// O2Jam's M30 XORing
// XOR sets of 4 bytes with mask. Remainder bytes are ignored.
static void decrypt_M30XOR (uint8_t *&sData, unsigned int sSize, const uint8_t *sMask)
{
    for ( unsigned int i = 0; i + 3 < sSize; i += 4 )
    {
        sData[i+0] ^= sMask[0];
        sData[i+1] ^= sMask[1];
        sData[i+2] ^= sMask[2];
        sData[i+3] ^= sMask[3];
    }
}

// O2Jam's OMC-WAV XORing
// Doesn't really XOR the data.
static void decrypt_accXOR (uint8_t *&sData, unsigned int sSize, bool reset = false)
{
    /**
     * The key byte is preserved throughout the whole file so it needs
     * to be defined as a static variable. It has to be reset whenever
     * a new file is parsed.
     */
    static uint32_t j = 0; /* byte counter */
    static uint32_t k = OMC_ACCKEY_INIT;

    uint8_t y , z; /* byte reserve */

    for ( unsigned int i = 0; i < sSize; i++ )
    {
        z = y = sData[i];

        if (((k << j) & 0x80) != 0)
            z = ~z;

        sData[i] = z;

        j++;
        if (j > 7) {
            j = 0;
            k = y;
        }
    }

    if (reset == true) {
        j = 0;
        k = OMC_ACCKEY_INIT;
    }
}

// O2Jam's OMC-WAV data shuffler
static void decrypt_arrange (uint8_t *&sData, unsigned int sSize)
{
    // rearrangement key
    unsigned int  k = ((sSize % 17) << 4) + (sSize % 17);

    // rearrangement block size
    unsigned int bs = sSize / 17;

    // temporary buffer
    uint8_t* sRawData = new uint8_t[sSize];
    std::copy (sData, sData + sSize, sRawData);
    for ( unsigned int b = 0; b < 17; b++ )
    {
        unsigned int se_bOffset = bs * b;                 // offset of encoded block
        unsigned int ed_bOffset = bs * c_Arrangement[k];  // offset of decoded block

        std::copy (sRawData + se_bOffset, sRawData + se_bOffset + bs, sData + ed_bOffset);
        k++;
    }

    delete[] sRawData;
}

// type M30 parser
void parseM30 (clan::File& file, SampleMap& sample_map)
{
    const int fileSize = file.get_size();
    const int headSize = sizeof(M30_File_Header);   // 32 - 4 (signature) = 28
    const int M30hSize = sizeof(M30_Sample_Header); // 52 bytes

    file.seek(0);

    // create read buffer
    uint8_t* buffer = new uint8_t[headSize];
    int read = file.read(buffer, headSize);
    if (read != headSize)
        delete[] buffer;

    const uint8_t *pPtr = buffer;

    // read header
    M30_File_Header *pFileHeader = (M30_File_Header*)pPtr;
    pPtr += headSize;

    uint32_t smplEncryption = pFileHeader->encryption;
    uint32_t smplCount      = pFileHeader->samples;
    uint32_t smplOffset     = pFileHeader->payload_addr;
    uint32_t packSize       = pFileHeader->payload_size;

    delete[] buffer;

    if (packSize > fileSize - smplOffset) {
        printf("[debug] Different payload size reported by header.\n");
        packSize = fileSize - smplOffset;
    }

    // Jump to payload location
    file.seek(smplOffset);

    // create read buffer
    buffer = new uint8_t[packSize];
    read = file.read(buffer, packSize);
    if (read != (int)packSize)
        delete[] buffer;

    pPtr = buffer;

    for (unsigned int i = 0; i < smplCount; i++)
    {
        // read sample header
        M30_Sample_Header *pSmplHeader = (M30_Sample_Header*)pPtr;
        pPtr += M30hSize;


        std::string smplName = pSmplHeader->name;
        smplName.append(".ogg");

        uint32_t smplSize = pSmplHeader->size;
        uint16_t smplType = pSmplHeader->type;
        uint16_t smplID   = pSmplHeader->id+1;

        uint8_t* pSmplData = (uint8_t*)pPtr;
        pPtr += smplSize;

        // decode sample
        switch (smplEncryption) {
            // unencrypted OGG
            case  0: break;
                     // namiXOR-ed OGG
            case 16: decrypt_M30XOR(pSmplData, smplSize, M30_nami_XORMASK); break;
                     // 0412XOR-ed OGG
            case 32: decrypt_M30XOR(pSmplData, smplSize, M30_0412_XORMASK); break;
        }

        // type M### note
        if (smplType == 0)
            smplID += 1000;


        // pass into OGG stream
        Sample* pSample = new Sample((char*)pSmplData, smplSize, smplName.c_str());

        if (pSample->getSource() == nullptr)
            printf("[warn] Failed to load M30 sample: %s\n", smplName.c_str());
        else
            sample_map[smplID] = pSample;

    }


    // clean up
    delete[] buffer;
}


// type OMC parser
void parseOMC (clan::File& file, bool isEncrypted, SampleMap& sample_map)
{
    // read headers
    long fileSize = file.get_size();
    const long headSize = sizeof(OMC_File_Header);
    const long WAVhSize = sizeof(OMC_WAV_Header);
    const long OGGhSize = sizeof(OMC_OGG_Header);

    file.seek(0);

    // create read buffer
    uint8_t* buffer = new uint8_t[headSize];
    int read = file.read(buffer, headSize);
    if (read != headSize)
        delete[] buffer;

    // read header
    const uint8_t *pPtr = (uint8_t*)((void*)buffer);
    OMC_File_Header *pFileHeader = (OMC_File_Header*)pPtr;
    pPtr += headSize;

    // Sample ID counter
    uint16_t smplID;

    uint32_t WAV_Offset = pFileHeader->wavs_addr;
    uint32_t OGG_Offset = pFileHeader->oggs_addr;
    uint32_t WAV_PackSize;
    uint32_t OGG_PackSize;

    // Calculate sound archive size. Usually the WAV archive comes first,
    // but just to be sure, we test the offsets.
    if (OGG_Offset > WAV_Offset) {          // WAVs first
        WAV_PackSize = OGG_Offset - WAV_Offset;
        OGG_PackSize =   fileSize - OGG_Offset;
    } else if (OGG_Offset < WAV_Offset){    // OGGs first
        WAV_PackSize =   fileSize - WAV_Offset;
        OGG_PackSize = WAV_Offset - OGG_Offset;
    } else {                                // no WAVs
        WAV_PackSize = 0;
        OGG_PackSize = fileSize - OGG_Offset;
    }

    // clear buffer
    delete[] buffer;


    if (WAV_PackSize > 0)
    {
        // parse WAV files
        // create new read buffer from offset
        file.seek(WAV_Offset);

        buffer = new uint8_t[WAV_PackSize];
        read = file.read(buffer, WAV_PackSize);
        if (read != (int)WAV_PackSize)
            delete[] buffer;

        pPtr = (uint8_t*)((void*)buffer);

        smplID = 0; // WAV

        unsigned long i = 0;

        while (i < WAV_PackSize)
        {
            // read WAV header
            OMC_WAV_Header *pWAVHeader = (OMC_WAV_Header*)pPtr;
            pPtr += WAVhSize;
            i += WAVhSize;
            smplID++;


            std::string SampleName(pWAVHeader->name);
            SampleName.append(".wav");

            /* Of all the things, why does it have to be a mangled header?
             * FIXME: Figure out why this doesn't work most of the time
             */
            uint16_t CodecFormat = pWAVHeader->fmt0_AudioFormat; // WAVE codec used
            uint16_t numChannels = pWAVHeader->fmt0_numChannels; // number of sample channels
            uint32_t SampleRate  = pWAVHeader->fmt0_SmplRate;    // samples per second (8000, 44100, etc.)
            uint32_t ByteRate    = pWAVHeader->fmt0_ByteRate;    //   bytes per second (SampleRate * FrameRate)
            uint16_t FrameRate   = pWAVHeader->fmt0_PlayRate;    //   bytes per sample frame [NumChannels * BitRate / 8]
            uint16_t BitRate     = pWAVHeader->fmt0_BitRate;     //    bits per sample (8, 16 or 32-bit)

            uint32_t SampleSize  = pWAVHeader->data_ChunkSize;   // size of actual sample data

            // ignore empty samples
            if (SampleSize > 0 || numChannels > 0)
            {
                // rip PCM data
                uint8_t *pSmplData = (uint8_t*)pPtr;
                pPtr += SampleSize;
                i += SampleSize;

                // decrypt data
                if (isEncrypted) {
                    decrypt_arrange(pSmplData, SampleSize);
                    decrypt_accXOR (pSmplData, SampleSize);
                }

                // create WAVE file buffer
                uint8_t* poSmplData = new uint8_t[SampleSize+44];

                WAV_Header* pWAVOutHead = (WAV_Header*)poSmplData;

                pWAVOutHead->RIFF_ID   = 0x46464952; // "RIFF"
                pWAVOutHead->RIFF_Size = SampleSize + 36;
                pWAVOutHead->RIFF_fmt0 = 0x45564157; // "WAVE"

                pWAVOutHead->fmt0_ID   = 0x20746d66; // "fmt "
                pWAVOutHead->fmt0_Size = 16;
                pWAVOutHead->fmt0_AudioFormat = CodecFormat;
                pWAVOutHead->fmt0_numChannels = numChannels;
                pWAVOutHead->fmt0_SmplRate  = SampleRate;
                pWAVOutHead->fmt0_ByteRate  = ByteRate;
                pWAVOutHead->fmt0_PlayRate  = FrameRate;
                pWAVOutHead->fmt0_BitRate   = BitRate;

                pWAVOutHead->data_ChunkID   = 0x61746164; // "data"
                pWAVOutHead->data_ChunkSize = SampleSize;

                // arrayCopy (pSmplData, 0, poSmplData, 44, SampleSize);
                std::copy (pSmplData, pSmplData + SampleSize, poSmplData + 44);
                // pass into WAVE stream
                Sample* pSample = new Sample((char*)pSmplData, SampleSize, SampleName.c_str());

                if (pSample->getSource() == nullptr)
                    printf("[warn] Failed to load OMC WAV sample: %s\n", SampleName.c_str());
                else
                    sample_map[smplID] = pSample;

                delete[] poSmplData;
            }
        }

        delete[] buffer;

    }

    /* reset accXOR */
    static uint8_t* null = 0;
    decrypt_accXOR(null, 0, true);

    if (OGG_PackSize > 0)
    {
        // parse OGG/MP3 files
        // create new read buffer from offset
        file.seek(OGG_Offset);

        buffer = new uint8_t[OGG_PackSize];
        read = file.read(buffer, OGG_PackSize);
        if (read != (int)OGG_PackSize)
            delete[] buffer;

        pPtr = (uint8_t*)((void*)buffer);

        smplID = 1000;

        unsigned long i = 0;

        while (i < OGG_PackSize)
        {
            // read OGG header
            OMC_OGG_Header *pOGGHeader = (OMC_OGG_Header*)pPtr;
            pPtr += OGGhSize;
            i += OGGhSize;

            smplID++;

            std::string SampleName = pOGGHeader->name; // already has extension
            uint32_t    SampleSize = pOGGHeader->size;

            if (SampleSize != 0) {
                // rip OGG/MP3 data
                uint8_t *pSmplData = (uint8_t*)pPtr;
                pPtr += SampleSize;
                i += SampleSize;

                // pass into OGG stream
                Sample* pSample = new Sample((char*)pSmplData, SampleSize, SampleName.c_str());

                if (pSample->getSource() == nullptr)
                    printf("[warn] Failed to load OMC OGG sample: %s\n", SampleName.c_str());
                else
                    sample_map[smplID] = pSample;

            }

        }
    }
}



void O2JamChart::load_samples()
{
    clan::File file;
    if (file.open(ojm_path,
                clan::File::OpenMode::open_existing,
                clan::File::AccessFlags::access_read
                ) == false)
        throw std::invalid_argument("Failed to open OJM file.");

    // create read buffer
    uint32_t* buffer = new uint32_t[4]();
    int read = file.read(buffer, 4);
    if (read != 4) {
        delete[] buffer;
        return;
    }

    // read signature
    uint32_t pSignature = *buffer;

    // clean up
    delete[] buffer;

    // test signature
    switch (pSignature)
    {
        case OJM_SIGNATURE: parseOMC(file, false, sample_map); break;
        case OMC_SIGNATURE: parseOMC(file, true , sample_map); break;
        case M30_SIGNATURE: parseM30(file, sample_map); break;
        default: printf("[warn] Unknown OJM signature. \n");
    }

    this->samples_loaded = true;
}

};

