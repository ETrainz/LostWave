add_definitions("-std=c++11")

add_library(awe STATIC
	aweLoop.cpp aweSample.cpp aweTrack.cpp awePortAudio.cpp awesndfile.cpp
	Filters/3BEQ.cpp Filters/Mixer.cpp)