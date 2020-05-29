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


#include <catch.hpp>

#include <TinyExpr.hpp>

TEST_CASE("TinyExpr simple")
{
    TinyExpr expr;

    int err = expr.Compile("(1+1)*2");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(4.0));
}

TEST_CASE("TinyExpr variables")
{
    TinyExpr expr;
    double x = 3;
    double y = 4.0;
    expr.AddVariable("x", &x);
    expr.AddVariable("y", &y);

    int err = expr.Compile("sqrt(x^2+y^2)");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(5.0));
}

static double func1(double arg)
{
    return arg;
}

TEST_CASE("TinyExpr function1")
{
    TinyExpr expr;
    expr.AddFunction("foo", func1);

    int err = expr.Compile("foo(1)");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(1.0));
}

static double func2(double arg1, double arg2)
{
    return arg1 + arg2;
}

TEST_CASE("TinyExpr function2")
{
    TinyExpr expr;
    expr.AddFunction("foo", func2);

    int err = expr.Compile("foo(1, 3)");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(4.0));
}

TEST_CASE("TinyExpr lambda")
{
    TinyExpr expr;
    // No captures!
    expr.AddFunction("foo", [](double arg) -> double
    {
        return arg * 2;
    });

    int err = expr.Compile("foo(1)");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(2.0));
}

TEST_CASE("TinyExpr lambda variable")
{
    TinyExpr expr;
    double x = 5;
    expr.AddVariable("x", &x);
    // No captures!
    expr.AddFunction("foo", [](double arg) -> double
    {
        return arg * 2;
    });

    int err = expr.Compile("foo(x)");
    REQUIRE(err == 0);
    double res = expr.Evaluate();
    REQUIRE(res == Approx(10.0));
}
