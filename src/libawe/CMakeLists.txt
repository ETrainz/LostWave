add_definitions("-std=c++11")

add_library(awe STATIC
    aweLoop.cpp awePortAudio.cpp
    Sources/Sample.cpp  Sources/awesndfile.cpp  Sources/Track.cpp
    Filters/3BEQ.cpp    Filters/IIR.cpp         Filters/Metering.cpp    Filters/Mixer.cpp)
