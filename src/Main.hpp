//  Main.hpp :: ClanLib application definition
//  Copyright 2013 Keigen Shu

#include <ClanLib/application.h>

class App
{
public:
    static int main(std::vector<std::string> const &args);
};

// ClanLib application boot location.
clan::Application app(&App::main);
