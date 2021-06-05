#ifndef _MENCI_CHILDPROCESS_H
#define _MENCI_CHILDPROCESS_H

#include <cstdio>
#include <string>
#include <vector>

class ChildProcess {
private:
    int pipeWrite;
    FILE *fileRead;
    bool exited;

public:
    pid_t pid;

    ChildProcess(const std::string &command, const std::vector<std::string> &arguments);
    ChildProcess(ChildProcess &&);
    ChildProcess(const ChildProcess &) = delete;
    ~ChildProcess();

    int wait(bool noHang = false);

    void write(const std::vector<char> &data);
    void write(const std::string &data);
    void write(const void *data, size_t length);
    void writeLine(const std::string &data);

    std::vector<char> read(size_t size);
    void read(std::vector<char> &result, size_t length);
    void read(std::string &result, size_t length);
    void read(void *buffer, size_t length);
    std::string readLine();
};

#endif // _MENCI_CHILDPROCESS_H
