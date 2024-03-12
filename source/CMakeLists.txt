# Set the appropriate compiler flags.
if(MSVC)
	target_compile_options(EndlessSkyEditor PUBLIC "/W3" "/permissive-" "/sdl" "/GR-" "/analyze-"
		"-Wno-nonportable-include-path" "$<$<CONFIG:Release>:/Gy;/Oi>")
	target_compile_definitions(EndlessSkyEditor PUBLIC "_UNICODE" "UNICODE")
else()
	target_compile_options(EndlessSkyEditor PUBLIC
		"-Wall" "-pedantic-errors" "-Wold-style-cast" "-fno-rtti")
endif()

# Every source file (and header file) should be listed here, except main.cpp.
# If you add a new file, add it to this list.
target_sources(EndlessSkyEditor PRIVATE
	AsteroidField.cpp
	AsteroidField.h
	CMakeLists.txt
	DataFile.cpp
	DataFile.h
	DataNode.cpp
	DataNode.h
	DataWriter.cpp
	DataWriter.h
	DetailView.cpp
	DetailView.h
	Galaxy.cpp
	Galaxy.h
	GalaxyView.cpp
	GalaxyView.h
	LandscapeLoader.cpp
	LandscapeLoader.h
	LandscapeView.cpp
	LandscapeView.h
	main.cpp
	MainWindow.cpp
	MainWindow.h
	Map.cpp
	Map.h
	pi.h
	Planet.cpp
	Planet.h
	PlanetView.cpp
	PlanetView.h
	SpriteSet.cpp
	SpriteSet.h
	StellarObject.cpp
	StellarObject.h
	System.cpp
	System.h
	SystemView.cpp
	SystemView.h
)
