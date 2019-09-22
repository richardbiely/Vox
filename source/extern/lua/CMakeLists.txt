file(GLOB LUA_SRCS *.h *.c)
set(LUA_SRCS ${LUA_SRCS} PARENT_SCOPE)

source_group("lua" FILES ${LUA_SRCS})
