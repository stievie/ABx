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
#include "TemplateEvaluator.h"
#include <TinyExpr.hpp>
#include <abshared/Attributes.h>
#include "Actor.h"
#include <sstream>

TemplateEvaluator::TemplateEvaluator(Actor& actor) :
    actor_(actor)
{
}

std::string TemplateEvaluator::Evaluate(const std::string& source)
{
    // Syntax:
    // This awesome skill heals by ${Healing * 2 + 3} and makes ${Death * 3} damage.
    TinyExpr expr;
    // Unfortunately we can not use captures with this implementation, so just
    // add the attribute ranks as variable.
#define ENUMERATE_ATTRIBUTE(v)                                                   \
    double v = static_cast<double>(actor_.GetAttributeRank(Game::Attribute::v)); \
    expr.AddVariable(#v, &v);
    ENUMERATE_ATTRIBUTES
#undef ENUMERATE_ATTRIBUTE

    std::stringstream result;
    TemplateParser parser;
    auto tokens = parser.Parse(source);
    for (auto& token : tokens)
    {
        switch (token.type)
        {
        case TemplateParser::TokenType::Expression:
        {
            double val = expr.Evaluate(token.value);
            // FIXME: Should we round instead of just truncating?
            result << static_cast<int>(val);
            break;
        }
        case TemplateParser::TokenType::String:
            result << token.value;
            break;
        case TemplateParser::TokenType::Invalid:
            break;
        }
    }

    return result.str();
}

TemplateParser::Token TemplateParser::GetNextToken()
{
    Token result;
    result.type = TokenType::Invalid;
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
                if (result.type != TokenType::Invalid)
                {
                    result.end = index_ - 1;
                    result.value = std::string(&source_[result.start], result.end - result.start);
                    --index_;
                    return result;
                }
                result.type = TokenType::Expression;
                result.start += 2;
                ++index_;
            }
            break;
        }
        case '}':
            result.end = index_ - 1;
            result.value = std::string(&source_[result.start], result.end - result.start);
            return result;
        case '\0':
            result.end = index_;
            result.value = std::string(&source_[result.start], result.end - result.start);
            return result;
        default:
            if (result.type == TokenType::Invalid)
                result.type = TokenType::String;
            break;
        }
    }
    result.end = index_;
    result.value = std::string(&source_[result.start], result.end - result.start);
    return result;
}

std::vector<TemplateParser::Token> TemplateParser::Parse(std::string_view source)
{
    source_ = source;
    std::vector<TemplateParser::Token> result;

    while (!Eof())
        result.push_back(GetNextToken());
    return result;
}
