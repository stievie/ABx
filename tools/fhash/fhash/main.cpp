/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <sa/ArgParser.h>
#include <absync/Partition.h>
#include <absync/Hash.h>
#include <filesystem>
#include <sa/time.h>
#include <fstream>
#include <absync/SyncDefs.h>

namespace fs = std::filesystem;

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << sa::arg_parser::get_help("fhash", _cli, "Calculate file block hashes");
    std::cout << std::endl;
    std::cout << "If no directory is given, it uses the current directory" << std::endl;
}

static void ShowInfo()
{
    std::cout << "fhash - Calculate file block hashes" << std::endl;
    std::cout << "(C) 2020, Stefan Ascher" << std::endl << std::endl;
}

static void ProcessEntry(const fs::directory_entry& p)
{
    if (p.is_directory())
        return;
    if (p.path().extension().string() == std::string(Sync::META_FILE_EXT) || p.path().extension().string() == std::string(Sync::HASH_FILE_EXT))
        return;

    const std::string filename = p.path().string();
    if (filename.empty())
        return;
    if ((filename != "..") &&
        (filename != ".") &&
        ((filename[0] == '.') || (filename.find("/.") != std::string::npos) || (filename.find("\\.") != std::string::npos)))
        return;

    std::cout << "Processing file " << filename << std::endl;
    const auto hashes = Sync::PartitionFile(filename, {});
    if (hashes.size() == 0)
        // File with 0 bytes
        return;
    Sync::SaveBoundaryList(filename, hashes);
    Sync::HashFile(filename, filename + Sync::HASH_FILE_EXT);
}

template<typename T>
static void ProcessDirectory(const T& iterator)
{
    for (const auto& p : iterator)
        ProcessEntry(p);
}

int main(int argc, char** argv)
{
    ShowInfo();
    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "--help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "recursive", { "-R", "--recursive" }, "Processs also subdirectores", false, false, sa::arg_parser::option_type::none }
    } };

    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return EXIT_SUCCESS;
    }

    if (!cmdres)
        return EXIT_FAILURE;

    auto recursive = sa::arg_parser::get_value<bool>(parsedArgs, "recursive", false);

    auto dirarg = sa::arg_parser::get_value<std::string>(parsedArgs, "0");
    fs::path dir = dirarg.has_value() ? fs::path(dirarg.value()) : fs::current_path();

    std::cout << "Directory " << dir.string() << std::endl;
    sa::time::timer timer;

    if (recursive)
        ProcessDirectory(fs::recursive_directory_iterator(dir));
    else
        ProcessDirectory(fs::directory_iterator(dir));

    std::cout << "Took " << timer.elapsed_seconds() << " seconds" << std::endl;
    return EXIT_SUCCESS;
}
