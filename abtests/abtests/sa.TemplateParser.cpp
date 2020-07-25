#include <catch.hpp>

#include <sa/TemplateParser.h>
#include <sa/Assert.h>

TEST_CASE("TemplateParser parse")
{
    std::string s1 = "SELECT * FROM table WHERE item_flags & ${ItemFlag} = ${ItemFlag} ORDER BY type DESC, name ASC";
    sa::TemplateParser parser;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        if (token.value == "ItemFlag")
            return std::to_string(4);
        return "???";
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT * FROM table WHERE item_flags & 4 = 4 ORDER BY type DESC, name ASC") == 0);
}

TEST_CASE("TemplateParser string escape")
{
    std::string s1 = "SELECT * FROM table WHERE name = ${name} ORDER BY type DESC, name ASC";
    sa::TemplateParser parser;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        if (token.value == "name")
            return "'a name'";
        return "???";
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT * FROM table WHERE name = 'a name' ORDER BY type DESC, name ASC") == 0);
}

TEST_CASE("TemplateParser invalid param")
{
    std::string s1 = "SELECT * FROM table WHERE name = ${invalid} ORDER BY type DESC, name ASC";
    sa::TemplateParser parser;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        if (token.value == "name")
            return "'a name'";
        return "???";
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT * FROM table WHERE name = ??? ORDER BY type DESC, name ASC") == 0);
}

TEST_CASE("TemplateParser quotes")
{
    std::string s1 = "SELECT FROM `table` WHERE `name` = ${name} ORDER BY `type` DESC, `name` ASC";
    sa::TemplateParser parser;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::Token::Type::Expression:
            if (token.value == "name")
                return "'a name'";
            return "???";
        case sa::Token::Type::Quote:
            return "'";
        default:
            return "";
        }
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT FROM 'table' WHERE 'name' = 'a name' ORDER BY 'type' DESC, 'name' ASC") == 0);
}

TEST_CASE("TemplateParser nested quotes")
{
    std::string s1 = "SELECT FROM `table` WHERE `nam\"e\"` = ${name} ORDER BY `type` DESC, `name` ASC";
    sa::TemplateParser parser;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::Token::Type::Expression:
            if (token.value == "name")
                return "'a name'";
            return "???";
        case sa::Token::Type::Quote:
            return "'";
        default:
            return "";
        }
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT FROM 'table' WHERE 'nam\"e\"' = 'a name' ORDER BY 'type' DESC, 'name' ASC") == 0);
}

TEST_CASE("TemplateParser no quotes")
{
    std::string s1 = "SELECT FROM `table` WHERE `nam\"e\"` = ${name} ORDER BY `type` DESC, `name` ASC";
    sa::TemplateParser parser;
    parser.quotesSupport_ = false;
    auto templ = parser.Parse(s1);
    templ.onEvaluate_ = [](const sa::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::Token::Type::Expression:
            if (token.value == "name")
                return "'a name'";
            return "???";
        case sa::Token::Type::Quote:
            ASSERT_FALSE();
        default:
            return "";
        }
    };
    std::string result = templ.ToString();

    REQUIRE(result.compare("SELECT FROM `table` WHERE `nam\"e\"` = 'a name' ORDER BY `type` DESC, `name` ASC") == 0);
}
