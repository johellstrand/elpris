set(CMAKE_CXX_STANDARD 20)

add_executable( elpris elpris.cpp ReadOnlyFileMMap.h )
target_link_libraries( elpris  curl )
