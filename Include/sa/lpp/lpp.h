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

#pragma once

/*
 * LPP
 *
 * Parses some sort of template language and emits reguilar Lua code.
 * It is a bit like PHP, but without PHP.
 *
 * Lua: <% for i = 10,1,-1 do %>
 * Shortcut for Lua print(): <%= 1 + 1 %>
 * Include a file: <? "test.lpp" ?>
 */

#include <string>
#include <vector>
#include <string_view>
#include <functional>
#include <iostream>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#if !defined (WRITE_FUNC)
#define WRITE_FUNC "io.write"
#endif

namespace sa {
namespace lpp {

namespace details {
template <typename charType>
std::basic_string<charType> Trim(const std::basic_string<charType>& str,
    const std::basic_string<charType>& whitespace = " \t")
{
    // Left
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return std::basic_string<charType>(); // no content

    // Right
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

template <typename charType>
bool ReplaceSubstring(std::basic_string<charType>& subject,
    const std::basic_string<charType>& search,
    const std::basic_string<charType>& replace)
{
    if (search.empty())
        return false;
    bool result = false;

    using string_type = std::basic_string<charType>;
    string_type newString;
    newString.reserve(subject.length());

    size_t lastPos = 0;
    size_t findPos;

    while ((findPos = subject.find(search, lastPos)) != string_type::npos)
    {
        newString.append(subject, lastPos, findPos - lastPos);
        newString += replace;
        lastPos = findPos + search.length();
        result = true;
    }

    newString += subject.substr(lastPos);

    subject.swap(newString);
    return result;
}

inline std::string EscapeLiteral(const std::string& value)
{
    std::string result = value;
    ReplaceSubstring<char>(result, "[", "\\[");
    ReplaceSubstring<char>(result, "]", "\\]");
    return result;
}
}

class Tokenizer;

struct Token
{
    enum class Type
    {
        Invalid,
        Literal,
        Code,
        Include,
        Print
    };

    Type type;
    size_t start;
    size_t end;
    std::string value;
    bool once { false };
};

using Tokens = std::vector<Token>;

class Tokenizer
{
private:
    size_t index_{ 0 };
    std::string_view source_;
    bool Eof() const { return index_ >= source_.size(); }
    char Next() { return source_.at(index_++); }
    char Peek(size_t offset = 1) const
    {
        if (source_.size() > index_ + offset)
            return source_.at(index_ + offset);
        return 0;
    }
    static void ParseInclude(Token& token)
    {
        token.value = details::Trim(token.value);
        details::ReplaceSubstring(token.value, std::string("\""), std::string(""));
    }
    Token GetNextToken()
    {
        Token result;
        result.type = Token::Type::Invalid;
        result.start = index_;
        while (!Eof())
        {
            char c = Next();
            switch (c)
            {
            case '<':
            {
                char c2 = Peek(0);
                if (c2 == '%')
                {
                    if (result.type != Token::Type::Invalid)
                    {
                        result.end = index_ - 1;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        --index_;
                        return result;
                    }
                    if (Peek(1) == '=')
                    {
                        result.type = Token::Type::Print;
                        ++index_;
                        result.start += 3;
                    }
                    else
                    {
                        result.type = Token::Type::Code;
                        result.start += 2;
                    }
                    ++index_;
                }
                else if (c2 == '?')
                {
                    if (result.type != Token::Type::Invalid)
                    {
                        result.end = index_ - 1;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        --index_;
                        return result;
                    }
                    result.type = Token::Type::Include;
                    result.start += 2;
                    if (Peek(1) == '#')
                    {
                        ++index_;
                        ++result.start;
                        result.once = true;
                    }
                    ++index_;
                }
                break;
            }
            case '%':
            {
                char c2 = Peek(0);
                if (c2 == '>')
                {
                    if (result.type == Token::Type::Code || result.type == Token::Type::Print)
                    {
                        result.end = index_ - 1;
                        ++index_;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        return result;
                    }
                }
                break;
            }
            case '?':
            {
                char c2 = Peek(0);
                if (c2 == '>')
                {
                    if (result.type == Token::Type::Include)
                    {
                        result.end = index_ - 1;
                        ++index_;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        ParseInclude(result);
                        return result;
                    }
                }
                break;
            }
            case '\0':
                result.end = index_;
                result.value = std::string(&source_[result.start], result.end - result.start);
                return result;
            default:
                if (result.type == Token::Type::Invalid)
                    result.type = Token::Type::Literal;
                break;
            }
        }
        result.end = index_;
        result.value = std::string(&source_[result.start], result.end - result.start);
        return result;
    }
    void AddToken(Tokens& tokens, Token&& token, std::vector<std::string>& includes)
    {
        if (token.type == Token::Type::Include)
        {
            if (token.once && IsIncluded(includes, token.value))
                return;
            if (onGetFile_)
            {
                includes.push_back(token.value);
                std::string src = onGetFile_(token.value);
                Tokenizer tokenizer;
                tokenizer.onGetFile_ = onGetFile_;
                tokenizer.Append(src, tokens, includes);
            }
            return;
        }
        tokens.push_back(std::move(token));
    }
    void Append(std::string_view source, Tokens& tokens, std::vector<std::string>& includes)
    {
        source_ = source;
        index_ = 0;
        while (!Eof())
            AddToken(tokens, GetNextToken(), includes);
    }
    static bool IsIncluded(std::vector<std::string>& includes, const std::string& filename)
    {
        const auto it = std::find_if(includes.begin(), includes.end(), [&filename](const auto& current)
        {
            return filename.compare(current) == 0;
        });
        return it != includes.end();
    }
    std::vector<std::string> includes_;
public:
    Tokens Parse(std::string_view source)
    {
        source_ = source;
        index_ = 0;
        Tokens result;
        while (!Eof())
            AddToken(result, GetNextToken(), includes_);
        return result;
    }
    std::function<std::string(const std::string&)> onGetFile_;
};

template<typename Callback>
inline void Generate(const Tokens& tokens, Callback&& callback)
{
    for (const auto& token : tokens)
    {
        switch (token.type)
        {
        case Token::Type::Invalid:
            break;
        case Token::Type::Print:
            callback(WRITE_FUNC"(" + details::Trim(token.value, std::string(" \t\r\n")) + ")");
            break;
        case Token::Type::Literal:
        {
            // Use print() to print literals:
            // https://stackoverflow.com/questions/4508119/redirecting-redefining-print-for-embedded-lua
            if (!token.value.empty())
                callback(WRITE_FUNC"([[" + details::EscapeLiteral(token.value) + "]])");
            break;
        }
        case Token::Type::Code:
            callback(token.value);
            break;
        case Token::Type::Include:
            break;
        }
    }
}

inline bool Run(lua_State* L, const std::string& source)
{
    if (!L)
        return false;

    if (luaL_dostring(L, source.c_str()) != 0)
    {
        size_t len = static_cast<size_t>(luaL_len(L, -1));
        const std::string err(lua_tostring(L, -1), len);
        std::cerr << err << std::endl;
        return false;
    }

    return true;
}

inline bool Run(const std::string& source)
{
    lua_State* L;
    L = luaL_newstate();
    if (!L)
        return false;
    luaL_openlibs(L);
    bool result = Run(L, source);
    lua_close(L);
    return result;
}

}
}
