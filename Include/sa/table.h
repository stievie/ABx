#pragma once

// Table format
//
// Usage:
/*
    sa::tab::table table;
    table << sa::tab::head << "head1" << sa::tab::endc << "head2" << sa::tab::endc << "head3" << sa::tab::endr;
    table << "test" << sa::tab::endc << "col2" << sa::tab::endc << sa::tab::ralign << "longer column" << sa::tab::endr;
    table << "second row" << sa::tab::endc << "col2" << sa::tab::endc << sa::tab::ralign << "col3";

    sa::tab::padding pad(table.width(), '=');
    std::cout << pad << std::endl << table << pad << std::endl;
  */
// Output:
/*
===============================
head1      head2 head3
-------------------------------
test       col2  longer column
second row col2           col3
===============================
 */

#include <vector>
#include <sstream>
#include <algorithm>

// I'm so sick of this!
#if defined(max)
#undef max
#endif

namespace sa {
namespace tab {

namespace details {

template<typename T, typename Tag>
struct escape_type
{
    constexpr escape_type() noexcept = default;
    constexpr escape_type(T value) noexcept :
        value_(value)
    { }
    operator T () const
    {
        return value_;
    }
    T value_;
};

typedef escape_type<char, struct end_row> end_row_type;
typedef escape_type<char, struct end_col> end_col_type;
typedef escape_type<char, struct left_align_tag> left_align_type;
typedef escape_type<char, struct right_align_tag> right_align_type;
typedef escape_type<char, struct heading_tag> heading_type;

}

static constexpr details::end_row_type endr{ '_' };
static constexpr details::end_col_type endc{ '|' };
static constexpr details::left_align_type lalign{ '<' };
static constexpr details::right_align_type ralign{ '>' };
static constexpr details::heading_type head{ '*' };

class row;
class table;

class padding
{
public:
    constexpr padding(size_t count) noexcept :
        count_(count)
    { }
    constexpr padding(size_t count, char value) noexcept :
        count_(count),
        value_(value)
    { }
    size_t count_;
    char value_{ ' ' };
};

class col
{
private:
    row& parent_;
public:
    enum class align
    {
        left,
        right
    };
    col(row& parent) :
        parent_(parent)
    { }
    col(const col& other) :
        parent_(other.parent_),
        align_(other.align_)
    {
        content_ << other.content_.str();
    }
    col& operator << (details::end_col_type);
    col& operator << (details::left_align_type)
    {
        align_ = align::left;
        return *this;
    }
    col& operator << (details::right_align_type)
    {
        align_ = align::right;
        return *this;
    }
    row& operator << (details::end_row_type);
    template<typename T>
    col& operator << (const T& v)
    {
        content_ << v;
        return*this;
    }
    size_t length() const { return content_.str().size(); }
    bool empty() const { return content_.str().length() == 0; }
    std::stringstream content_;
    align align_{ align::left };
};

class row
{
    friend class table;
private:
    table& parent_;
    std::vector<col> cols_;
public:
    row(table& parent) :
        parent_(parent)
    { }
    row(const row& other) :
        parent_(other.parent_),
        cols_(other.cols_),
        heading_(other.heading_)
    { }
    row& operator << (col&& c)
    {
        cols_.push_back(std::move(c));
        return *this;
    }
    row& operator << (details::heading_type)
    {
        heading_ = true;
        return *this;
    }
    col& operator << (details::end_col_type&)
    {
        col c(*this);
        cols_.push_back(c);
        return cols_.back();
    }
    row& operator << (details::end_row_type);
    template<typename T>
    col& operator << (const T& v)
    {
        cols_.back() << v;
        return cols_.back();
    }
    size_t count() const { return cols_.size(); }
    bool empty() const { return count() == 0 || (count() == 1 && cols_.front().empty()); }
    size_t width() const
    {
        size_t result = 0;
        for (const auto& c : cols_)
            result = std::max(result, c.length());
        return result;
    }
    const col& operator[](size_t i) const { return cols_.at(i); }
    bool heading_{ false };
};

class table
{
private:
    std::vector<row> rows_;
public:
    table()
    {
        rows_.push_back(row(*this));
        rows_.back().cols_.push_back(col(rows_.back()));
    }
    table& operator << (row&& r)
    {
        rows_.push_back(std::move(r));
        return *this;
    }
    row& operator << (details::heading_type v)
    {
        return rows_.back() << v;
    }
    row& operator << (details::end_row_type)
    {
        row r(*this);
        rows_.push_back(r);
        rows_.back().cols_.push_back(col(rows_.back()));
        return rows_.back();
    }
    template<typename T>
    col& operator << (const T& v)
    {
        return rows_.back().cols_.back() << v;
    }
    size_t count() const { return rows_.size(); }
    const row& operator[](size_t i) const { return rows_.at(i); }
    size_t col_width(size_t i) const
    {
        size_t max_w = 0;
        for (const auto& r : rows_)
        {
            if (i < r.count())
            {
                const col& c = r[i];
                max_w = std::max(max_w, c.length());
            }
        }
        return max_w;
    }
    size_t width() const
    {
        size_t result = 0;
        for (const auto& r : rows_)
        {
            size_t row_width = 0;
            for (size_t i = 0; i < r.cols_.size(); ++i)
            {
                row_width += col_width(i);
                // Col Separator
                ++row_width;
            }
            result = std::max(result, row_width);
        }
        return result;
    }
};

inline col& col::operator << (details::end_col_type v)
{
    return parent_ << v;
}

inline row& col::operator << (details::end_row_type v)
{
    return parent_ << v;
}

inline row& row::operator << (details::end_row_type v)
{
    return parent_ << v;
}

template<class _Stream>
inline _Stream& operator << (_Stream& os, table& value)
{
    for (size_t i = 0; i < value.count(); ++i)
    {
        const auto& r = value[i];
        for (size_t j = 0; j < r.count(); ++j)
        {
            const col& c = r[j];
            size_t max_w = value.col_width(j);
            size_t c_w = c.length();
            int pad = static_cast<int>(max_w) - static_cast<int>(c_w);
            if (c.align_ == col::align::left)
                os << c.content_.str();
            if (pad > 0)
            {
                padding p(static_cast<size_t>(pad));
                os << p;
            }
            if (c.align_ == col::align::right)
                os << c.content_.str();
            if (j < r.count() - 1)
                // If not the last column add a col separator
                os << ' ';
        }
        os << std::endl;
        if (r.heading_)
        {
            size_t w = value.width();
            for (size_t sepi = 0; sepi < w; ++sepi)
                os << '-';
            os << std::endl;
        }
    }
    return os;
}

template<class _Stream>
inline _Stream& operator << (_Stream& os, padding& value)
{
    for (size_t i = 0; i < value.count_; ++i)
        os << value.value_;
    return os;
}

}
}
