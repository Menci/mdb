#include "ChildProcess.h"

#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Ensure/Ensure.h"

ChildProcess::ChildProcess(const std::string &command, const std::vector<std::string> &arguments) {
    exited = false;

    int pipeTo[2], pipeFrom[2];
    ENSURE_ERRNO(pipe(pipeTo));
    ENSURE_ERRNO(pipe(pipeFrom));

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    posix_spawn_file_actions_adddup2(&actions, pipeTo[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipeFrom[1], STDOUT_FILENO);

    std::vector<const char *> argv(arguments.size() + 2);
    argv[0] = command.c_str();
    for (size_t i = 0; i < arguments.size(); i++) argv[i + 1] = arguments[i].c_str();
    argv.back() = nullptr;
    
    ENSURE_ZERO_ERRNO(posix_spawnp(&pid, command.c_str(), &actions, nullptr, const_cast<char *const *>(argv.data()), nullptr));

    posix_spawn_file_actions_destroy(&actions);

    ENSURE_ERRNO(close(pipeTo[0]));
    ENSURE_ERRNO(close(pipeFrom[1]));

    fileRead = fdopen(pipeFrom[0], "r");
    pipeWrite = pipeTo[1];
}
  
ChildProcess::ChildProcess(ChildProcess &&other) {
    pid = other.pid;
    fileRead = other.fileRead;
    pipeWrite = other.pipeWrite;
    exited = other.exited;

    other.pid = 0;
}

ChildProcess::~ChildProcess() {
    if (pid) {
        if (!exited) {
            kill(pid, SIGKILL);
            wait();
        }
        ENSURE_ERRNO(fclose(fileRead));
        ENSURE_ERRNO(close(pipeWrite));
    }
}

int ChildProcess::wait(bool noHang) {
    int status;
    waitpid(pid, &status, noHang ? WNOHANG : 0);
    if (WIFEXITED(pid) || WIFSIGNALED(pid)) exited = true;
    return status;
}

void ChildProcess::write(const std::vector<char> &data) {
    write(data.data(), data.size());
}

void ChildProcess::write(const std::string &data) {
    write(data.data(), data.size());
}

void ChildProcess::write(const void *data, size_t length) {
    ENSURE_ERRNO(::write(pipeWrite, data, length));
}

void ChildProcess::writeLine(const std::string &data) {
    write(data + "\n");
}

std::vector<char> ChildProcess::read(size_t size) {
    std::vector<char> buffer(size);
    read(buffer.data(), size);
    return buffer;
}

void ChildProcess::read(std::vector<char> &result, size_t length) {
    result.resize(length);
    read(result.data(), length);
}

void ChildProcess::read(std::string &result, size_t length) {
    result.resize(length);
    read(result.data(), length);
}

void ChildProcess::read(void *buffer, size_t length) {
    ENSURE_ERRNO(fread(buffer, length, 1, fileRead));
}

std::string ChildProcess::readLine() {
    char *p = nullptr;
    size_t size = 0;
    ENSURE_ERRNO(getline(&p, &size, fileRead));
    std::string result = p ? p : "";
    if (result.length() > 0) result.pop_back();
    free(p);
    return result;
}
