#include <iostream>
#include <sa/ArgParser.h>

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << std::endl;
    std::cout << sa::arg_parser::get_help("dbgclient", _cli);
}

int main(int argc, char** argv)
{
    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none },
        { "host", { "-host" }, "Server host", true, true, sa::arg_parser::option_type::string },
        { "port", { "-p", "-port" }, "Server port", true, true, sa::arg_parser::option_type::integer }
    } };

    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return EXIT_SUCCESS;
    }
    if (!cmdres)
    {
        std::cout << cmdres << std::endl;
        std::cout << "Type `dbgclient -h` for help." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
