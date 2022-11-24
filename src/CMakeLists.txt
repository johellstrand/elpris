set(CMAKE_CXX_STANDARD 20)


#find_package( gumbo-parser )
include_directories( /opt/homebrew/opt/gumbo-parser/include/ )
link_directories( /opt/homebrew/opt/gumbo-parser/lib/ )

add_executable( elpris elpris.cpp )
target_link_libraries( elpris gumbo curl )
