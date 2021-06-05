project(ensure CXX)

file(GLOB SRC "*.cc")

add_library(ensure SHARED ${SRC})
