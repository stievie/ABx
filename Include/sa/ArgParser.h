/**
 * Copyright 2017-2020 Stefan Ascher
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

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <sa/Compiler.h>
#include <sstream>
#include <string>
#include <vector>

namespace sa {
namespace arg_parser {

constexpr char NAME_VALUE_DELIM = '=';

enum class option_type
{
    none,
    string,
    integer,
    number                         // float
};

struct option
{
    std::string name;
    std::vector<std::string> switches;
    std::string help;
    bool mandatory;
    bool has_argument;
    option_type type{ option_type::none };
};

struct value_base { };

template <typename T>
struct value : public value_base
{
    T value;
};

typedef std::map<std::string, std::unique_ptr<value_base>> values;

template <typename T>
inline std::optional<T> get_value(const values& vals, const std::string& name)
{
PRAGMA_WARNING_PUSH
    // GCC: This warning is wrong
PRAGMA_WARNING_DISABLE_GCC("-Wmaybe-uninitialized")
    const auto it = vals.find(name);
    if (it == vals.end())
        return {};
    const auto* v = static_cast<value<T>*>((*it).second.get());
    return v->value;
PRAGMA_WARNING_POP
}

template <typename T>
inline T get_value(const values& vals, const std::string& name, T def)
{
    const auto it = vals.find(name);
    if (it == vals.end())
        return def;
    const auto* v = static_cast<value<T>*>((*it).second.get());
    return v->value;
}

// Command line interface
typedef std::vector<option> cli;

inline void remove_option(const std::string& name, cli& _cli)
{
    auto it = std::find_if(_cli.begin(), _cli.end(), [&name](const option& current) {
        return current.name.compare(name) == 0;
    });
    if (it != _cli.end())
        _cli.erase(it);
}

// Parsing result
struct result
{
    bool success{ false };
    std::vector<std::string> items;
    operator bool() noexcept { return success; }
};

inline result parse(const std::vector<std::string>& args, const cli& _cli, values& vals)
{
    result res;
    res.success = true;

    auto is_int = [](const std::string& s) -> bool
    {
        if (s.empty())
            return false;
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return it == s.end();
    };
    auto is_float = [](const std::string& s) -> bool
    {
        if (s.empty())
            return false;
        std::istringstream iss(s);
        float f;
        iss >> std::noskipws >> f;
        return iss.eof() && !iss.fail();
    };

    auto find_option = [&](const std::string& name) -> std::optional<option>
    {
        for (const auto& o : _cli)
        {
            for (const auto& s : o.switches)
            {
                if (s.compare(name) == 0)
                    return o;
            }
        }
        return { };
    };

    auto split_name_value = [](const std::string& s) -> std::pair<std::string, std::string>
    {
        auto pos = s.find(NAME_VALUE_DELIM);
        if (pos != std::string::npos)
            return { s.substr(0, pos), s.substr(pos + 1) };
        return { s, "" };
    };

    int unnamed = 0;
    for (auto it = args.begin(); it != args.end(); ++it)
    {
        // Split -name=value
        auto name_value = split_name_value(*it);
        auto o = find_option(name_value.first);

        if (!o.has_value())
        {
            // This option is not defined in the CLI, add is as unnamed.
            std::unique_ptr<value<std::string>> val = std::make_unique<value<std::string>>();
            val->value = (*it);
            vals.emplace(std::to_string(unnamed), std::move(val));
            ++unnamed;
            continue;
        }

        if (o->has_argument)
        {
            switch (o->type)
            {
            case option_type::none:
                // Argument with type none?
                res.success = false;
                break;
            case option_type::string:
            {
                if (name_value.second.empty())
                {
                    ++it;
                    if (it == args.end())
                    {
                        res.success = false;
                        res.items.push_back("Missing argument for '" + o->name + "'");
                    }
                }
                if (res.success)
                {
                    std::unique_ptr<value<std::string>> val = std::make_unique<value<std::string>>();
                    if (name_value.second.empty())
                        val->value = (*it);
                    else
                        val->value = name_value.second;
                    vals.emplace(o->name, std::move(val));
                }
                break;
            }
            case option_type::integer:
            {
                if (name_value.second.empty())
                {
                    ++it;
                    if (it == args.end())
                    {
                        res.success = false;
                        res.items.push_back("Missing argument for '" + o->name + "'");
                    }
                    if (res.success && !is_int(*it))
                    {
                        res.success = false;
                        res.items.push_back("Expected integer type but got '" + (*it) +  + "' for '" + o->name + "'");
                    }
                }
                else
                {
                    if (!is_int(name_value.second))
                    {
                        res.success = false;
                        res.items.push_back("Expected integer type but got '" + name_value.second +  + "' for '" + o->name + "'");
                    }
                }
                if (res.success)
                {
                    std::unique_ptr<value<int>> val = std::make_unique<value<int>>();
                    if (name_value.second.empty())
                        val->value = std::atoi((*it).c_str());
                    else
                        val->value = std::atoi(name_value.second.c_str());
                    vals.emplace(o->name, std::move(val));
                }
                break;
            }
            case option_type::number:
            {
                if (name_value.second.empty())
                {
                    ++it;
                    if (it == args.end())
                    {
                        res.success = false;
                        res.items.push_back("Missing argument for '" + o->name + "'");
                    }
                    if (res.success && !is_float(*it))
                    {
                        res.success = false;
                        res.items.push_back("Expected float type but got '" + (*it) + "' for '" + o->name + "'");
                    }
                }
                else
                {
                    if (!is_float(name_value.second))
                    {
                        res.success = false;
                        res.items.push_back("Expected float type but got '" + name_value.second + "' for '" + o->name + "'");
                    }
                }
                if (res.success)
                {
                    std::unique_ptr<value<float>> val = std::make_unique<value<float>>();
                    if (name_value.second.empty())
                        val->value = static_cast<float>(std::atof((*it).c_str()));
                    else
                        val->value = static_cast<float>(std::atof(name_value.second.c_str()));
                    vals.emplace(o->name, std::move(val));
                }
                break;
            }
            }
        }
        else
        {
            // No argument -> set this flag to true
            std::unique_ptr<value<bool>> val = std::make_unique<value<bool>>();
            val->value = true;
            vals.emplace(o->name, std::move(val));
        }
        if (!res.success)
            break;
    }

    if (res.success)
    {
        // Check if all mandatory options are given
        int i = 0;
        for (const auto& o : _cli)
        {
            if (!o.mandatory)
                continue;

            if (o.switches.size() != 0)
            {
                if (vals.find(o.name) == vals.end())
                {
                    res.items.push_back("Required argument '" + o.name + "' is missing");
                    res.success = false;
                }
            }
            else
            {
                // Mandatory unnamed argument
                if (vals.find(std::to_string(i)) == vals.end())
                {
                    res.items.push_back("Required positional argument '" + o.name + "' is missing");
                    res.success = false;
                }
                ++i;
            }
        }
    }
    return res;
}

// Parse the arguments according the command line interface and put the
// result in vals
inline result parse(int argc, char** argv, const cli& _cli, values& vals)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    return parse(args, _cli, vals);
}

struct help
{
    std::vector<std::string> lines;
};

enum class help_format
{
    plain,
    markdown,
};

// Generate help for the given command line interface
inline help get_help(const std::string& prog, const cli& arg,
    const std::string description = "",
    help_format format = help_format::plain)
{
    auto get_type_string = [](const option& o) -> std::string
    {
        switch (o.type)
        {
        case option_type::none:
            return o.name;
        case option_type::string:
            return "string";
        case option_type::integer:
            return "integer";
        case option_type::number:
            return "number";
        default:
            return o.name;
        }
    };

    auto get_heading = [&](const std::string& heading) -> std::string
    {
        switch (format)
        {
        case help_format::markdown:
            return "## " + heading + "\n";
        default:
            return heading;
        }
    };

    auto get_code = [&](const std::string& value) -> std::string
    {
        switch (format)
        {
        case help_format::markdown:
            return "`" + value + "`";
        default:
            return value;
        }
    };

    auto get_synopsis = [&]() -> std::string
    {
        std::stringstream synopsis;
        synopsis << prog << " ";
        for (const auto& o : arg)
        {
            std::stringstream sw;
            if (!o.mandatory)
                sw << "[";
            if (o.switches.size() != 0)
            {
                sw << o.switches.front();
                if (o.has_argument)
                    sw << " ";
            }
            if (o.has_argument)
                sw << "<" << o.name << ">";
            if (!o.mandatory)
                sw << "]";
            sw << " ";
            synopsis << sw.str();
        }

        switch (format)
        {
        case help_format::markdown:
            return "~~~sh\n$ " + synopsis.str() + "\n~~~";
        default:
            return "    " + synopsis.str();
        }
    };

    auto get_switches_line = [&](const std::string& switches)
    {
        switch (format)
        {
        case help_format::markdown:
            return switches;
        default:
            return "    " + switches;
        }
    };

    help result;

    if (!description.empty())
    {
        result.lines.push_back(description);
        result.lines.push_back("");
    }
    if (!prog.empty())
    {
        result.lines.push_back(get_heading("SYNOPSIS"));
        result.lines.push_back(get_synopsis());
        result.lines.push_back("");
    }
    result.lines.push_back(get_heading("OPTIONS"));
    std::vector<std::pair<std::string, std::string>> shp;
    size_t maxLength = 0;
    size_t maxHelp = 0;
    for (const auto& o : arg)
    {
        std::string switches;
        if (!o.mandatory)
            switches += "[";
        if (o.switches.size() != 0)
        {
            for (const auto& a : o.switches)
            {
                if (switches.size() > 1)
                    switches += ", ";
                switches += get_code(a);
            }
        }
        else
        {
            switches += o.name;
        }
        if (o.has_argument)
            switches += " <" + get_type_string(o) + ">";
        if (!o.mandatory)
            switches += "]";
        std::string line = get_switches_line(switches);
        shp.push_back({ line, o.help });
        if (maxLength < line.size())
            maxLength = line.size();
        if (maxHelp < o.help.length())
            maxHelp = o.help.length();
    }

    if (format == help_format::plain)
    {
        bool wrap = maxLength + maxHelp > 72;
        for (const auto& l : shp)
        {
            std::string f = l.first;

            if (wrap)
            {
                // Wrap long lines
                result.lines.push_back(l.first);
                result.lines.push_back("        " + l.second);
            }
            else
            {
                f.insert(f.size(), maxLength - f.size(), ' ');
                result.lines.push_back(f + "   " + l.second);
            }
        }
    }
    else if (format == help_format::markdown)
    {
        for (const auto& l : shp)
        {
            std::string f = l.first;
            result.lines.push_back("* " + l.first + ": " + l.second);
        }
    }

    return result;
}

template<class _Stream>
inline _Stream& operator << (_Stream& os, const help& help)
{
    for (const auto& line : help.lines)
    {
        os << line;
        os << '\n';
    }
    return os;
}

template<class _Stream>
inline _Stream& operator << (_Stream& os, const result& res)
{
    if (!res.success)
    {
        for (const auto& line : res.items)
        {
            os << line;
            os << '\n';
        }
    }
    return os;
}

}
}
