#include <cstdio>
#include <string>
#include <algorithm>
#include "Chart_BMS.hpp"

#include <ClanLib/core.h>
#include "Music.hpp"


uint string_to_raw_uint(const std::string &str)
{
    uint v = 0;
    for(uint i = 0; i < str.size(); ++i)
        v += static_cast<uint>(str[i]) << (i*8);
    return v;
}

uint repeating_char_to_raw_uint(char c, uint l)
{
    std::string str;
    while(l --> 0) str += c;
    return string_to_raw_uint(str);
}

static bool get_bms_token(const std::string &str, const std::string &search, std::string &token)
{
    auto i = str.find(search);
    if (i == std::string::npos) return false;
    if (i >= str.size()) return false;
    auto a = str.cbegin() + i + search.size();

    while (*a == '\t' || *a == ' ')
        if (++a == str.cend())
            return false;

    auto b = str.cend() + str.find_last_not_of("\t ") - (str.size() - 1);
    token = std::string(a, b);
    return true;
}

static bool get_bms_token(const std::string &str, const std::string &search, std::string &t, std::string &v)
{
    auto i = str.find(search);
    if (i == std::string::npos) return false;
    if (i >= str.size()) return false;
    auto a = str.cbegin() + i + search.size();

    while (*a == '\t' || *a == ' ')
        if (++a == str.cend())
            return false;

    auto b = str.cend() + str.find_last_not_of("\t ") - (str.size() - 1);
    std::string s (a, b);

    i = s.find_first_of("\t ");
    if (i == std::string::npos)
        return false;

    a = b = s.cbegin() + i;
    while (*b == '\t' || *b == ' ')
        if (++b == str.cend())
            return false;

    t = std::string(s.cbegin(), a);
    v = std::string(b, s.cend());

    return true;
}

static bool get_bms_token(const std::string &str, std::string &t, std::string &v)
{
    auto i = str.find(':');
    if (i == std::string::npos) return false;
    if (i >= str.size()) return false;
    auto a = str.cbegin() + i;
    auto b = str.cend() + str.find_last_not_of("\t ") - (str.size() - 1);
    t = std::string(str.cbegin() + 1, a);
    v = std::string(a + 1, b);

    return true;
}

void Chart_BMS::init_sequence()
{
    this->clear();

    for(uint m = 0; m <= measures; m++)
    {
        auto it = measure_ts_z.find(m);
        if (it == measure_ts_z.end()) {
            sequence.push_back(new Measure(4, 48));
        } else {
            sequence.push_back(new Measure(it->second));
        }
    }
}

typedef std::list<Note_Single*> SingleNoteList;

Chart_BMS::Chart_BMS(const std::string &path) :
    Chart(),
    bms_fullpath(clan::PathHelp::get_fullpath(path)),
    bms_filename(clan::PathHelp::get_filename(path)),
    // TODO Detect ID length
    WAV_ID_LENGTH(0),
    BMP_ID_LENGTH(2),
    BPM_ID_LENGTH(2),
    STP_ID_LENGTH(2),
    measures(0)
{
    {   /* Input and filtering stage */

        std::string full_script(clan::File::read_text(path));

        // Read into vector and remove line endings
        for(auto a = full_script.cbegin(), b = full_script.cbegin(); b != full_script.cend(); ++b)
        {
            if (*b == '\n') {
                if (*(b-1) == '\r') { // DOS-style line endings
                    if (a != b-1)
                        script.push_back(std::string(a, b-1));
                    else
                        script.push_back(std::string());
                } else { // Unix-style line endings
                    if (a != b)
                        script.push_back(std::string(a, b));
                    else
                        script.push_back(std::string());
                }

                a = b + 1;
            }
        }
    } { /* Read header and pre-parse chart data */


        for(size_t i = 0; i < script.size(); i++)
        {
            std::string line = script[i];

            auto p = line.find_first_not_of("\t ");
            if (p == std::string::npos)
                continue; // empty line

            if (line[p] != '#')
                continue; // first char is not '#'

            // ignore whitespace at end of command
            auto q = line.find_last_not_of("\t ");
            if (q == std::string::npos) {
                q = 0;
            } else {
                assert(p < q);
                q = line.size() - (q+1);
            }

            line = std::string(line.cbegin() + p, line.cend() - q);

            /* If you got here, then you look like a legitimate command! */
            // TODO: Make this case-insensitive
            std::string t, v;

            // Measure size command list, to be used after pre-parse

            /****/ if (get_bms_token(line, "#PLAYER", t)) {     // HEAD::Chart type
                type = clan::StringHelp::text_to_uint(t);
            } else if (get_bms_token(line, "#GENRE", t)) {      // HEAD::Chart genre
                genre = t;
            } else if (get_bms_token(line, "#TITLE", t)) {      // HEAD::Chart title
                name = t;
            } else if (get_bms_token(line, "#ARTIST", t)) {     // HEAD::Chart/music artist
                charter = t;
            } else if (get_bms_token(line, "#PLAYLEVEL", t)) {  // HEAD::Level
                level = clan::StringHelp::text_to_uint(t);
            } else if (get_bms_token(line, "#RANK", t)) {       // HEAD::Difficulty category
                rank = clan::StringHelp::text_to_uint(t);
            } else if (get_bms_token(line, "#VOLWAV", t)) {     // HEAD::Master output volume
                vol = clan::StringHelp::text_to_double(t);
            } else if (get_bms_token(line, "#STAGEFILE", t)) {  // HEAD::Loading art
                stage_file = t;
            } else if (get_bms_token(line, "#BPM", t, v)) {     // BODY::BPM value statement (Read this first!)
                bpms[string_to_raw_uint(t)] = clan::StringHelp::text_to_double(v);
            } else if (get_bms_token(line, "#BPM", t)) {        // HEAD::Tempo
                tempo = clan::StringHelp::text_to_double(t);
            } else if (get_bms_token(line, "#STOP", t, v)) {    // BODY::STOP value statement
                stops[string_to_raw_uint(t)] = clan::StringHelp::text_to_uint(v);
            } else if (get_bms_token(line, "#WAV", t, v)) {     // BODY::WAV file statement
                wavs[string_to_raw_uint(t)] = v;
                if (WAV_ID_LENGTH == 0) {
                    WAV_ID_LENGTH = t.size();
                } else if (WAV_ID_LENGTH != t.size()) {
                    const uchar l = std::max(std::string::size_type(WAV_ID_LENGTH), t.size());
                    printf("BMS [warn] Line %lu:\n%s\n", i, line.c_str());
                    printf("    [--->] Irregular WAV ID length. was %u, now %lu -> use %u.\n", WAV_ID_LENGTH, t.size(), l);
                    WAV_ID_LENGTH = l;
                }

            } else if (get_bms_token(line, t, v)) {             // BODY::Note data statement
                if (t.size() != 5) {
                    printf("BMS [warn] Ignoring line %lu:\n%s\n", i, line.c_str());
                    printf("    [--->] First part of the note statement must be 5 characters long.\n");
                    continue;
                }

                uint m = clan::StringHelp::text_to_uint(t.substr(0,3));
                std::string c = t.substr(3,2);

                measures = std::max(m, measures);

                if (c.compare("02") == 0) { // Measure beat size command
                    double u = clan::StringHelp::text_to_double(v);
                    measure_ts_z[m] = u;
                } else { // Everything else should be a note-command
                    messages.emplace(std::make_pair(i, std::make_pair(t,v)));
                }
            } else {
                printf("BMS [warn] Ignoring line %lu:\n%s\n", i, line.c_str());
                printf("    [--->] Failed to parse command.\n");
            }
        }
    }
}



void Chart_BMS::load_art()
{
    if (stage_file.empty())
        return;

    clan::PixelBuffer cover = clan::ImageProviderFactory::load(bms_fullpath + stage_file);
    if (cover.is_null())
        return;

    setCoverArt(cover.to_format(clan::tf_rgb8));
}

void Chart_BMS::load_chart()
{
    this->init_sequence();
    printf("WAV_ID_LENGTH = %u\n", WAV_ID_LENGTH);
    printf("BMP_ID_LENGTH = %u\n", BMP_ID_LENGTH);
    printf("BPM_ID_LENGTH = %u\n", BPM_ID_LENGTH);
    printf("STP_ID_LENGTH = %u\n", STP_ID_LENGTH);
    SingleNoteList LongNotes;

    for(auto p : messages)
    {
        auto line = p.first;
        auto mesg = p.second;

        auto head = mesg.first;
        auto data = mesg.second;

        uint mn = clan::StringHelp::text_to_uint(head.substr(0,3));
        uint ch = clan::StringHelp::text_to_uint(head.substr(3,2));

        auto m = sequence[mn];

        if (data.size() % WAV_ID_LENGTH != 0) {
            printf("BMS [warn] Ignoring line %u:\n%s\n", line, data.c_str());
            printf("    [--->] Data string length is not a multiple of WAV_ID_LENGTH assumed from sources.\n");
            printf("    [--->] Data string length = %lu <---> WAV_ID_LENGTH = %u\n", data.size(), WAV_ID_LENGTH);
            continue;
        }

/*
01 : BGM(background music by WAVE)
03 : changing a Tempo
04 : BGA(background animation)
06 : changing Poor-bitmap
11 to 17 : Object Channel of 1 player side
21 to 27 : Object Channel of 2 player side
*/
        if (ch == 3) { // BPM Change >> Read as uchar
            // BM98: This channel always uses 2-character hexadecimal >> uint.
            uint factor = m->getTickCount() / (data.size() / 2);

            for(size_t i=0; i<data.size() / 2; ++i)
            {
                uint bpm = clan::StringHelp::text_to_uint(data.substr(i*2, 2), 16);
                if (bpm != 0)
                {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    ParamEvent* p = new ParamEvent(time, EParam::EP_C_TEMPO, double(bpm));
                    m->addParamEvent(p);
                    printf(
                            "BMS [info] Added BPM_D event at %u:%u:%u [%lf]\n",
                            p->time.measure,
                            p->time.beat,
                            p->time.tick,
                            p->value.asFloat
                          );
                }
            }
        } else if (ch == 4 || ch == 6 || ch == 7) { // TODO Implement BGA

        } else if (ch == 8) { // BPM Change using reference table
            uint factor = m->getTickCount() / (data.size() / BPM_ID_LENGTH);

            for(size_t i=0; i<data.size() / BPM_ID_LENGTH; ++i)
            {
                std::string str = data.substr(i*BPM_ID_LENGTH, BPM_ID_LENGTH);
                uint bpm = string_to_raw_uint(str);

                if (bpm == repeating_char_to_raw_uint('0', BPM_ID_LENGTH)) {
                    continue;
                } else if (bpms.find(bpm) == bpms.end()) {
                    printf("BMS [warn] Error on line %u:%s\n", line, data.c_str());
                    printf("    [--->] BPM %s is undefined.\n", str.c_str());
                    continue;
                } else {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    ParamEvent *p = new ParamEvent(time, EParam::EP_C_TEMPO, double(bpms[bpm]));
                    m->addParamEvent(p);
                    printf(
                            "BMS [info] Added BPM_L event at %u:%u:%u [%s]->%lf\n",
                            p->time.measure,
                            p->time.beat,
                            p->time.tick,
                            str.c_str(),
                            p->value.asFloat
                          );
                }
            }
        } else if (ch == 9) { // STOP via lookup table
            uint factor = m->getTickCount() / (data.size() / STP_ID_LENGTH);
            for(size_t i=0; i<data.size() / STP_ID_LENGTH; ++i)
            {
                std::string str = data.substr(i*STP_ID_LENGTH, STP_ID_LENGTH);
                uint stop = string_to_raw_uint(str);

                if (stop == repeating_char_to_raw_uint('0', STP_ID_LENGTH)) {
                    continue;
                } else if (stops.find(stop) == stops.end()) {
                    printf("BMS [warn] Error on line %u:%s\n", line, data.c_str());
                    printf("    [--->] STOP %s is undefined.\n", str.c_str());
                    continue;
                } else {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    ParamEvent* p = new ParamEvent(time, EParam::EP_C_STOP_T, int64_t(stops[stop]));
                    m->addParamEvent(p);
                    printf(
                            "BMS [info] Added STOP event at %u:%u:%u [%s]->%li\n",
                            p->time.measure,
                            p->time.beat,
                            p->time.tick,
                            str.c_str(),
                            p->value.asInt
                          );
                }
            }
        } else if (ch == 1) { // Background notes
            uint factor = m->getTickCount() / (data.size() / WAV_ID_LENGTH);

            for(size_t i=0; i<data.size() / WAV_ID_LENGTH; ++i)
            {
                uint wav = string_to_raw_uint(data.substr(i*WAV_ID_LENGTH, WAV_ID_LENGTH));
                if (wav != repeating_char_to_raw_uint('0', WAV_ID_LENGTH))
                {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    Note_Single* n = new Note_Single(ENKey::NOTE_AUTO, time, wav);
                    m->addNote(n);
                }
            }
        } else if (ch > 10 && ch < 30) { // Normal notes
            ENKey key;
            switch (ch) {
                case 11: key = ENKey::NOTE_P1_1; break;
                case 12: key = ENKey::NOTE_P1_2; break;
                case 13: key = ENKey::NOTE_P1_3; break;
                case 14: key = ENKey::NOTE_P1_4; break;
                case 15: key = ENKey::NOTE_P1_5; break;
                case 16: key = ENKey::NOTE_P1_S; break;
                case 18: key = ENKey::NOTE_P1_6; break;
                case 19: key = ENKey::NOTE_P1_7; break;

                case 21: key = ENKey::NOTE_P2_1; break;
                case 22: key = ENKey::NOTE_P2_2; break;
                case 23: key = ENKey::NOTE_P2_3; break;
                case 24: key = ENKey::NOTE_P2_4; break;
                case 25: key = ENKey::NOTE_P2_5; break;
                case 26: key = ENKey::NOTE_P2_S; break;
                case 28: key = ENKey::NOTE_P2_6; break;
                case 29: key = ENKey::NOTE_P2_7; break;

                // TODO preserve mapping
                default: key = ENKey::NOTE_AUTO; break;
            }

            uint factor = m->getTickCount() / (data.size() / WAV_ID_LENGTH);
            for(size_t i=0; i<data.size() / WAV_ID_LENGTH; ++i)
            {
                uint wav = string_to_raw_uint(data.substr(i*WAV_ID_LENGTH, WAV_ID_LENGTH));
                if (wav != repeating_char_to_raw_uint('0', WAV_ID_LENGTH))
                {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    Note_Single* n = new Note_Single(key, time, wav);
                    m->addNote(n);
                }
            }

        } else if (ch >= 30 && ch <  50) { // Invisible notes

        } else if (ch >= 50 && ch <  70) { // Long notes
            ENKey key;
            switch (ch)
            {
                case 51: key = ENKey::NOTE_P1_1; break;
                case 52: key = ENKey::NOTE_P1_2; break;
                case 53: key = ENKey::NOTE_P1_3; break;
                case 54: key = ENKey::NOTE_P1_4; break;
                case 55: key = ENKey::NOTE_P1_5; break;
                case 56: key = ENKey::NOTE_P1_S; break;
                case 58: key = ENKey::NOTE_P1_6; break;
                case 59: key = ENKey::NOTE_P1_7; break;

                case 61: key = ENKey::NOTE_P2_1; break;
                case 62: key = ENKey::NOTE_P2_2; break;
                case 63: key = ENKey::NOTE_P2_3; break;
                case 64: key = ENKey::NOTE_P2_4; break;
                case 65: key = ENKey::NOTE_P2_5; break;
                case 66: key = ENKey::NOTE_P2_S; break;
                case 68: key = ENKey::NOTE_P2_6; break;
                case 69: key = ENKey::NOTE_P2_7; break;

                // TODO preserve mapping
                default: key = ENKey::NOTE_AUTO; break;
            }

            uint factor = m->getTickCount() / (data.size() / WAV_ID_LENGTH);
            for(size_t i=0; i<data.size() / WAV_ID_LENGTH; ++i)
            {
                uint wav = string_to_raw_uint(data.substr(i*WAV_ID_LENGTH, WAV_ID_LENGTH));
                if (wav != repeating_char_to_raw_uint('0', WAV_ID_LENGTH))
                {
                    TTime time = m->getTimeFromTickCount(i*factor); time.measure = mn;
                    Note_Single* n = new Note_Single(key, time, wav);
                    LongNotes.push_back(n);
                }
            }
        } else {
            printf("Unknown channel: %u read from string #%s:%s\n", ch, head.c_str(), data.c_str());
        }

    }
    printf("Sorting long notes...\n");
    LongNotes.sort(cmpNote_Greater);

    printf("Connecting %lu long notes...\n", LongNotes.size());

    while (LongNotes.size() > 2)
    {
        auto it = LongNotes.begin();
        Note_Single *a = *it; it++;
        Note_Single *b = *it;

        while(b->getKey() != a->getKey())
            b = *(++it);

        Note_Long *p = new Note_Long(*a, *b);
        sequence[p->getTime().first.measure]->addNote(p);

        LongNotes.erase(LongNotes.begin());
        LongNotes.erase(it);
        delete a;
        delete b;
    }

}

void Chart_BMS::load_samples()
{
    for(auto def : wavs)
    {
        std::string file = bms_fullpath;
        file = clan::PathHelp::add_trailing_slash(file);
        file.append(def.second);
        Sample* sample = new Sample(file.c_str());
        if (sample->getSource() == nullptr)
            printf("[warn] Failed to load sample: %s\n", file.c_str());
        else
            sample_map[def.first] = sample;
    }
}

Music* scan_BMS_directory(const std::string &path)
{
    clan::DirectoryScanner clDS;
    Music* music = new Music { path };
    size_t index = 0;

    if (clDS.scan(path, "*.bms"))
    {
        while(clDS.next())
        {
            std::string file = std::string(path) + "/" + clDS.get_name().c_str();
            Chart_BMS* chart = new Chart_BMS(file);
            music->title  = chart->getName();
            music->artist = chart->getCharter();
            music->genre  = chart->getGenre();
            music->charts[index] = chart;
            index++;
        }
    }

    if (index == 0)
    {
        delete music;
        music = nullptr;
    }

    return music;
}