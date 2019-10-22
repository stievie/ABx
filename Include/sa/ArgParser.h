#pragma once

#include <vector>
#include <string>
#include <optional>
#include <map>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <memory>

namespace sa {
namespace arg_parser {

enum class option_type
{
    none,
    string,
    integer,
    number                         // float
};

struct option
{
    const std::string name;
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
    auto it = vals.find(name);
    if (it == vals.end())
        return {};
    const auto* v = static_cast<value<T>*>((*it).second.get());
    return v->value;
}

template <typename T>
inline T get_value(const values& vals, const std::string& name, T def)
{
    auto it = vals.find(name);
    if (it == vals.end())
        return def;
    const auto* v = static_cast<value<T>*>((*it).second.get());
    return v->value;
}

typedef std::vector<option> cli;

inline bool parse(const std::vector<std::string>& args, const cli& _cli, values& result)
{
    bool success = true;

    auto is_int = [](const std::string& s) -> bool
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    };
    auto is_float = [](const std::string& s) -> bool
    {
        std::istringstream iss(s);
        float f;
        iss >> std::skipws >> f;
        return (f && iss.eof());
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

    int unnamed = 0;
    for (auto it = args.begin(); it != args.end(); ++it)
    {
        auto o = find_option((*it));
        if (!o.has_value())
        {
            std::unique_ptr<value<std::string>> val = std::make_unique<value<std::string>>();
            val->value = (*it);
            result.emplace(std::to_string(unnamed), std::move(val));
            ++unnamed;
            continue;
        }

        if (o->has_argument)
        {
            ++it;
            switch (o->type)
            {
            case option_type::none:
                // Argument with type none?
                --it;
                success = false;
                break;
            case option_type::string:
            {
                if (it != args.end())
                {
                    std::unique_ptr<value<std::string>> val = std::make_unique<value<std::string>>();
                    val->value = (*it);
                    result.emplace(o->name, std::move(val));
                }
                else
                    success = false;
                break;
            }
            case option_type::integer:
            {
                if (it != args.end() && is_int(*it))
                {
                    std::unique_ptr<value<int>> val = std::make_unique<value<int>>();
                    val->value = std::atoi((*it).c_str());
                    result.emplace(o->name, std::move(val));
                }
                else
                    success = false;
                break;
            }
            case option_type::number:
            {
                if (it != args.end() && is_float(*it))
                {
                    std::unique_ptr<value<float>> val = std::make_unique<value<float>>();
                    val->value = static_cast<float>(std::atof((*it).c_str()));
                    result.emplace(o->name, std::move(val));
                }
                else
                    success = false;
                break;
            }
            }
        }
        else
        {
            std::unique_ptr<value<bool>> val = std::make_unique<value<bool>>();
            val->value = true;
            result.emplace(o->name, std::move(val));
        }
        if (!success)
            break;
    }

    if (success)
    {
        // Check if all mandatory options are given
        for (const auto& o : _cli)
        {
            if (!o.mandatory)
                continue;

            if (result.find(o.name) == result.end())
            {
                success = false;
                break;
            }
        }
    }
    return success;
}

inline bool parse(int argc, char** argv, const cli& _cli, values& result)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    return parse(args, _cli, result);
}

struct help
{
    std::vector<std::string> lines;
};

inline help get_help(const std::string& prog, const cli& arg)
{
    help result;
    if (!prog.empty())
    {
        result.lines.push_back("SYNOPSIS");
        std::stringstream synopsis;
        synopsis << "    ";
        synopsis << prog << " ";
        for (const auto& o : arg)
        {
            std::stringstream sw;
            if (!o.mandatory)
                sw << "[";
            sw << o.switches.front();
            if (o.has_argument)
                sw << " <" << o.name << ">";
            if (!o.mandatory)
                sw << "]";
            synopsis << sw.str() << " ";
        }
        result.lines.push_back(synopsis.str());
        result.lines.push_back("");
    }
    result.lines.push_back("OPTIONS");
    std::vector<std::pair<std::string, std::string>> shp;
    size_t maxLength = 0;
    for (const auto& o : arg)
    {
        std::string switches;
        if (!o.mandatory)
            switches += "[";
        for (const auto& a : o.switches)
        {
            if (switches.size() > 1)
                switches += ", ";
            switches += a;
        }
        if (o.has_argument)
            switches += " <" + o.name + ">";
        if (!o.mandatory)
            switches += "]";
        std::string line = "    " + switches;
        shp.push_back({ line, o.help });
        if (maxLength < line.size())
            maxLength = line.size();
    }

    for (const auto& l : shp)
    {
        std::string f = l.first;
        f.insert(f.size(), maxLength - f.size(), ' ');
        result.lines.push_back(f + "   " + l.second);
    }
    return result;
}

template<class _Stream>
_Stream& operator << (_Stream& os, const help& help)
{
    for (const auto& line : help.lines)
    {
        os << line;
        os << '\n';
    }
    return os;
}

}
}
