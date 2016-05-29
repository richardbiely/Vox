file(GLOB GUI_SRCS *.cpp *.h)
set(GUI_SRCS ${GUI_SRCS} PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("gui" FILES ${GUI_SRCS})