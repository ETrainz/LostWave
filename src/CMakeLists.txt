find_package(ClanLib REQUIRED)

add_definitions("-std=c++11 -ggdb")

add_executable(LostWave
	AudioManager.cpp InputManager.cpp
	Chart.cpp Chart_BMS.cpp Chart_O2Jam.cpp Music.cpp MusicScanner.cpp
	Chrono.cpp JSON.cpp Measure.cpp Note.cpp
	UI/FFT.cpp
	UI/Tracker.cpp
	UI/MusicSelector
	Game.cpp # Game_Playing.cpp Game_Selection.cpp
	Main.cpp
)

include_directories(${ClanLib_INCLUDE_DIRS} ${awe_SOURCE_DIR} ${kiss_fft_SOURCE_DIR})
link_directories(${awe_BINARY_DIR} ${kiss_fft_BINARY_DIR})
target_link_libraries(LostWave
	sndfile portaudio awe kiss_fft
	${ClanLib_App_LIBRARY}
	${ClanLib_Core_LIBRARY}
	${ClanLib_Display_LIBRARY}
	${ClanLib_GL_LIBRARY}
	${ClanLib_CSSLayout_LIBRARY}
	${ClanLib_GUI_LIBRARY}
	${ClanLib_Network_LIBRARY}
	)
