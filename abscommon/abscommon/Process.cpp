#include "stdafx.h"
#include "Process.hpp"

namespace System {

Process::Process(const string_type &command, const string_type &path,
    std::function<void(const char* bytes, size_t n)> read_stdout,
    std::function<void(const char* bytes, size_t n)> read_stderr,
    bool open_stdin, size_t buffer_size) noexcept:
    closed(true),
        read_stdout(std::move(read_stdout)),
        read_stderr(std::move(read_stderr)),
        open_stdin(open_stdin),
        buffer_size(buffer_size)
{
    open(command, path);
    async_read();
}

Process::~Process() noexcept
{
    close_fds();
}

Process::id_type Process::get_id() const noexcept
{
    return data.id;
}

bool Process::write(const std::string &_data)
{
    return write(_data.c_str(), _data.size());
}

} // System
