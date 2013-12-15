//  Music.cpp :: Measures and Music sequences
//  Copyright 2011 - 2013 Keigen Shu

#include "Music.hpp"

void Music::clear_charts()
{
    for(ChartMapNode node : charts)
        delete node.second;

    charts.clear();
}
