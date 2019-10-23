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
