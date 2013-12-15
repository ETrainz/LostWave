//  Music.hpp :: Music object structure
//  Copyright 2011 - 2013 Keigen Shu

#ifndef MUSIC_H
#define MUSIC_H

#include <map>
#include <list>
#include <string>

#include "Chart.hpp"

typedef std::map <unsigned int, Chart*> ChartMap;
typedef std::pair<unsigned int, Chart*> ChartMapNode;

struct Music
{
    std::string path;

    std::string title;
    std::string artist;
    std::string genre;

    ChartMap    charts;

    void clear_charts();
};

typedef std::list<Music*> MusicList;


#endif
