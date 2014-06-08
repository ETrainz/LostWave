//  Main.hpp :: ClanLib application definition
//  Copyright 2013 Keigen Shu

#include <ClanLib/application.h>

class Game;
class Chart;

class App
{
public:
    static Game* game;

    static int main(std::vector<std::string> const &args);

    static void launchChart(Chart* chart);
};

// ClanLib application boot location.
clan::Application app(&App::main);
