set(CMAKE_CXX_STANDARD 23)


#find_package( gumbo-parser )
include_directories( /opt/homebrew/opt/gumbo-parser/include/ )
link_directories( /opt/homebrew/opt/gumbo-parser/lib/ )

add_executable( elpris elpris.cpp ReadOnlyFileMMap.h )
target_link_libraries( elpris gumbo curl )
