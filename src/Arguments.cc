#include "Arguments.h"

#include <ArgumentParser/ArgumentParser.h>

Arguments parseArguments(int argc, char *argv[]) {
    Arguments arguments;
    ArgumentParser(argc, argv)
        .setProgramDescription("Simple debugger with ptrace(2).")
        .addPositional(
            "executable",
            "The executable file of program being started and debugged.",
            ArgumentParser::stringParser(arguments.executable),
            true
        )
        .addOption(
            "pid", "p",
            "pid",
            "The pid of process to be attached and debugged.",
            ArgumentParser::integerParser<size_t>(arguments.pid),
            true, "0"
        )
        .parse();
    return arguments;
}
