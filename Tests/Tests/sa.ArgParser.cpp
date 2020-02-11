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

#include "stdafx.h"
#include <catch.hpp>

#include <sa/ArgParser.h>

TEST_CASE("ArgParser Success")
{
    sa::arg_parser::cli opts{ {
        {"inputfile", { "-i", "--input-file" }, "Input file", true, true, sa::arg_parser::option_type::string},
        {"outputfile", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string},
        {"number", { "-n", "--number" }, "A number", true, true, sa::arg_parser::option_type::integer},
        {"help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none}
    } };
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back("input_file");
    args.push_back("-o");
    args.push_back("output_file");
    args.push_back("-n");
    args.push_back("42");

    sa::arg_parser::values result;

    auto res = sa::arg_parser::parse(args, opts, result);
    REQUIRE(res.success);
    {
        auto val = sa::arg_parser::get_value<int>(result, "number");
        REQUIRE(val == 42);
    }
    {
        auto val = sa::arg_parser::get_value<std::string>(result, "inputfile");
        REQUIRE(val->compare("input_file") == 0);
    }
}

TEST_CASE("ArgParser multiple arg")
{
    sa::arg_parser::cli opts{ {
        {"outputfile", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string},
    } };
    std::vector<std::string> args;
    args.push_back("file1");
    args.push_back("file2");
    args.push_back("file3");
    args.push_back("file4");
    args.push_back("file5");

    sa::arg_parser::values result;

    auto res = sa::arg_parser::parse(args, opts, result);
    REQUIRE(res.success);
    REQUIRE(result.size() == 5);
}

TEST_CASE("ArgParser Fail")
{
    sa::arg_parser::cli opts{ {
        {"inputfile", { "-i", "--input-file" }, "Input file", true, true, sa::arg_parser::option_type::string},
        {"outputfile", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string},
        {"number", { "-n", "--number" }, "A number", true, true, sa::arg_parser::option_type::integer},
        {"help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none}
    } };
    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back("output_file");

    sa::arg_parser::values result;

    auto res = sa::arg_parser::parse(args, opts, result);
    REQUIRE(!res.success);
}

TEST_CASE("ArgParser Fail missing arg")
{
    sa::arg_parser::cli opts{ {
        {"inputfile", { "-i", "--input-file" }, "Input file", true, true, sa::arg_parser::option_type::string},
        {"outputfile", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string},
        {"number", { "-n", "--number" }, "A number", true, true, sa::arg_parser::option_type::integer},
        {"help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none}
    } };
    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back("output_file");
    args.push_back("-n");
    args.push_back("-i");

    sa::arg_parser::values result;

    auto res = sa::arg_parser::parse(args, opts, result);
    REQUIRE(!res.success);
}

TEST_CASE("ArgParser name=value")
{
    sa::arg_parser::cli opts{ {
        {"inputfile", { "-i", "--input-file" }, "Input file", true, true, sa::arg_parser::option_type::string},
        {"outputfile", { "-o", "--output-file" }, "Output file", false, true, sa::arg_parser::option_type::string},
        {"number", { "-n", "--number" }, "A number", true, true, sa::arg_parser::option_type::integer},
        {"help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none}
    } };
    std::vector<std::string> args;
    args.push_back("-i=input_file");
    args.push_back("-o=output_file");
    args.push_back("-n=42");

    sa::arg_parser::values result;

    auto res = sa::arg_parser::parse(args, opts, result);
    REQUIRE(res.success);
    {
        auto val = sa::arg_parser::get_value<int>(result, "number");
        REQUIRE(val == 42);
    }
    {
        auto val = sa::arg_parser::get_value<std::string>(result, "inputfile");
        REQUIRE(val->compare("input_file") == 0);
    }
}
