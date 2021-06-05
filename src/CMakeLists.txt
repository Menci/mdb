project(mdb CXX)

file(GLOB SRC "*.cc" "**/*.cc")
add_executable(mdb ${SRC})

target_link_libraries(
    mdb
    argument-parser ensure child-process regex-replace
    readline source-highlight Zydis
)
