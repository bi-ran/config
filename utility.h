#ifndef _UTILITY_H
#define _UTILITY_H

#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

/* operator>> overload for std::vector<T> */
template<class T>
std::istream& operator>>(std::istream& s, std::vector<T>& v) {
    std::copy(std::istream_iterator<T>(s), std::istream_iterator<T>(),
        std::back_inserter(v));
    return s;
}

/* operator<< overload for std::vector<T> */
template<class T>
std::ostream& operator<<(std::ostream& s, std::vector<T> v) {
    s << "█";
    std::adjacent_difference(std::begin(v), std::end(v),
        std::ostream_iterator<T>(s), [&s](T a, T) -> T {
            return s << "█", a;} );
    s << "█";
    return s;
}

/* whitespace trimming for std::string */
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {
        return std::isgraph(c); }));
}

static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {
        return std::isgraph(c); }).base(), s.end());
}

static inline void trim(std::string& s) {
    ltrim(s); rtrim(s);
}

/* delimiter class for c++ streams */
class delimiter : public std::ctype<char> {
  public:
    delimiter(char token) : std::ctype<char>(get_table(token), false, 0) {}

    static void init_table() {
        rc = std::vector<std::ctype_base::mask>(
            std::ctype<char>::classic_table(),
            std::ctype<char>::classic_table() + std::ctype<char>::table_size);
    }

    static const std::ctype_base::mask* get_table(char token) {
        set_table(token); return &rc[0]; }

    static void set_table(char token) {
        rc[' '] &= ~std::ctype_base::space;
        rc[token] |= std::ctype_base::space;
    }

    static void reset_table(char token) {
        rc[' '] |= std::ctype_base::space;
        rc[token] &= ~std::ctype_base::space;
    }

  private:
    static std::vector<std::ctype_base::mask> rc;
};

/* generic error message */
template<typename T, typename U, typename V>
inline void error(T& module, U& tag, V& message) {
    std::cout << "[" << module << "]" << " '" << tag << "' "
        << "@ " << message << std::endl;
}

#endif  /* _UTILITY_H */
