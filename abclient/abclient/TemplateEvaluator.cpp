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


#include "TemplateEvaluator.h"
#include <TinyExpr.hpp>
#include <abshared/Attributes.h>
#include "Actor.h"
#include <sstream>
#include <algorithm>
#include <sa/TemplateParser.h>

TemplateEvaluator::TemplateEvaluator(Actor& actor) :
    actor_(actor)
{
}

std::string TemplateEvaluator::Evaluate(const std::string& source)
{
    if (source.empty())
        return "";

    // Syntax:
    // This awesome skill heals by $(healing * 2 + 3} and makes ${death * 3} damage.
    // NOTE: Variables must be in lower case.
    TinyExpr expr;
    // Unfortunately we can not use lambdas with captures with this implementation, so just
    // add all attribute ranks as variable.
#define ENUMERATE_ATTRIBUTE(v)                                                   \
    double v = static_cast<double>(actor_.GetAttributeRank(Game::Attribute::v)); \
    expr.AddVariable(#v, &v);
    ENUMERATE_ATTRIBUTES
#undef ENUMERATE_ATTRIBUTE

    sa::TemplateParser parser;
    parser.quotesSupport_ = false;
    auto tokens = parser.Parse(source);
    return tokens.ToString([&expr](const sa::Token& token) -> std::string
    {
        if (token.type == sa::Token::Type::Expression)
            return std::to_string(static_cast<int>(expr.Evaluate(token.value)));
        return "";
    });
}
