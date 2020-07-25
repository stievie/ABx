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

#include <string>
#include <vector>
#include <string_view>
#include <sstream>
#include <functional>

namespace sa {

class TemplateParser;

struct Token
{
    enum class Type
    {
        Invalid,
        String,
        Expression,
        Quote
    };

    Type type;
    size_t start;
    size_t end;
    std::string value;
};

class Template
{
    friend class TemplateParser;
private:
    void AddToken(Token&& token)
    {
        tokens_.push_back(std::move(token));
    }
    std::vector<Token> tokens_;
public:
    bool IsEmpty() const { return tokens_.empty(); }
    std::string ToString()
    {
        std::stringstream ss;
        for (const auto& token : tokens_)
        {
            switch (token.type)
            {
            case Token::Type::Expression:
            case Token::Type::Quote:
                if (onEvaluate_)
                    ss << onEvaluate_(token);
                else
                    ss << token.value;
                break;
            case Token::Type::String:
                ss << token.value;
                break;
            default:
                break;
            }
        }
        return ss.str();
    }
    std::function<std::string(const Token& token)> onEvaluate_;
};

class TemplateParser
{
private:
    size_t index_{ 0 };
    char quote_{ '\0' };
    bool inQuote_{ false };
    std::string_view source_;
    bool Eof() const { return index_ >= source_.size(); }
    char Next() { return source_.at(index_++); }
    char Peek(size_t offset = 1) const
    {
        if (source_.size() > index_ + offset)
            return source_.at(index_ + offset);
        return 0;
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
            case '$':
            {
                char c2 = Peek(0);
                if (c2 == '{')
                {
                    if (result.type != Token::Type::Invalid)
                    {
                        result.end = index_ - 1;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        --index_;
                        return result;
                    }
                    result.type = Token::Type::Expression;
                    result.start += 2;
                    ++index_;
                }
                break;
            }
            case '}':
                if (result.type == Token::Type::Expression)
                {
                    result.end = index_ - 1;
                    result.value = std::string(&source_[result.start], result.end - result.start);
                    return result;
                }
                break;
            case '`':
            case '\'':
            case '"':
                if (inQuote_ && quote_ != c)
                    continue;
                if (result.type == Token::Type::String)
                {
                    result.end = index_ - 1;
                    result.value = std::string(&source_[result.start], result.end - result.start);
                    --index_;
                    return result;
                }
                result.type = Token::Type::Quote;
                result.end = index_ - 1;
                result.value = c;
                if (inQuote_)
                {
                    if (c == quote_)
                        inQuote_ = false;
                }
                else
                {
                    inQuote_ = true;
                    quote_ = c;
                }
                return result;
            case '\0':
                result.end = index_;
                result.value = std::string(&source_[result.start], result.end - result.start);
                return result;
            default:
                if (result.type == Token::Type::Invalid)
                    result.type = Token::Type::String;
                break;
            }
        }
        result.end = index_;
        result.value = std::string(&source_[result.start], result.end - result.start);
        return result;
    }
public:
    Template Parse(std::string_view source)
    {
        source_ = source;
        Template result;

        while (!Eof())
            result.AddToken(std::move(GetNextToken()));
        return result;
    }
};

}
