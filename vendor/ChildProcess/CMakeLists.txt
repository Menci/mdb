project(child-process CXX)

file(GLOB SRC "*.cc")

add_library(child-process SHARED ${SRC})
