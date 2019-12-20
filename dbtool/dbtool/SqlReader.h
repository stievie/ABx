#pragma once

#include <string>
#include <vector>
#include <sa/Iteration.h>

class SqlReader
{
private:
    std::vector<std::string> lines_;
public:
    SqlReader();
    bool Read(const std::string& filename);
    bool IsEmpty() const { return lines_.size() == 0; }
    template <typename Callback>
    void VisitStatements(const Callback& callback)
    {
        std::string statement;
        for (const auto& s : lines_)
        {
            // Concatenate lines until we find a semicolon at the end.
            statement += s;
            if (statement.back() != ';')
            {
                statement += ' ';
            }
            else
            {
                if (callback(statement) != Iteration::Continue)
                    break;
                statement.clear();
            }
        }
    }
};
