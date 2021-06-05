project(regex-replace CXX)

file(GLOB SRC "*.cc")

add_library(regex-replace SHARED ${SRC})
