#pragma once
#ifndef TINY_PROCESS_LIBRARY_HPP_
#define TINY_PROCESS_LIBRARY_HPP_

#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#ifdef AB_UNIX
#include <sys/wait.h>
#endif

namespace System {

///Platform independent class for creating processes
class Process
{
public:
#if defined(AB_WINDOWS)
    typedef unsigned long id_type; //Process id type
    typedef void *fd_type;         //File descriptor type
#elif defined(AB_UNIX)
    typedef pid_t id_type;
    typedef int fd_type;
#endif
    typedef std::string string_type;
private:
    class Data
    {
    public:
        Data() noexcept;
        id_type id;
#ifdef AB_WINDOWS
        void *handle;
#endif
    };
public:
    ///Note on Windows: it seems not possible to specify which pipes to redirect.
    ///Thus, at the moment, if read_stdout==nullptr, read_stderr==nullptr and open_stdin==false,
    ///the stdout, stderr and stdin are sent to the parent process instead.
    Process(const string_type &command, const string_type &path = string_type(),
        std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
        std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
        bool open_stdin = false,
        size_t buffer_size = 131072) noexcept;
#ifndef AB_WINDOWS
    /// Supported on Unix-like systems only.
    Process(std::function<void()> function,
        std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
        std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
        bool open_stdin = false,
        size_t buffer_size = 131072) noexcept;
#endif
    ~Process() noexcept;

    ///Get the process id of the started process.
    id_type get_id() const noexcept;
    ///Wait until process is finished, and return exit status.
    int get_exit_status() noexcept;
    ///If process is finished, returns true and sets the exit status. Returns false otherwise.
    bool try_get_exit_status(int &exit_status) noexcept;
    ///Write to stdin.
    bool write(const char *bytes, size_t n);
    ///Write to stdin. Convenience function using write(const char *, size_t).
    bool write(const std::string &data);
    ///Close stdin. If the process takes parameters from stdin, use this to notify that all parameters have been sent.
    void close_stdin() noexcept;

    ///Kill the process. force=true is only supported on Unix-like systems.
    void kill(bool force = false) noexcept;
    ///Kill a given process id. Use kill(bool force) instead if possible. force=true is only supported on Unix-like systems.
    static void kill(id_type id, bool force = false) noexcept;

    const Data& GetData() const
    {
        return data;
    }
private:
    Data data;
    bool closed;
    std::mutex close_mutex;
    std::function<void(const char* bytes, size_t n)> read_stdout;
    std::function<void(const char* bytes, size_t n)> read_stderr;
    std::thread stdout_thread, stderr_thread;
    bool open_stdin;
    std::mutex stdin_mutex;
    size_t buffer_size;

    std::unique_ptr<fd_type> stdout_fd, stderr_fd, stdin_fd;

    id_type open(const string_type &command, const string_type &path) noexcept;
#ifndef AB_WINDOWS
    id_type open(std::function<void()> function) noexcept;
#endif
    void async_read() noexcept;
    void close_fds() noexcept;
};

} // System

#endif  // TINY_PROCESS_LIBRARY_HPP_
