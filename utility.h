#ifndef _UTILITY_H
#define _UTILITY_H

#include <vector>
#include <string>
#include <functional>
#include <numeric>
#include <algorithm>
#include <iostream>

/* operator<< overload for std::vector<T> */
template<class T>
std::ostream& operator<<(std::ostream& s, std::vector<T> v) {
    s << "◆";
    adjacent_difference(begin(v), end(v), std::ostream_iterator<T>(s), [&s](T a, T)->T { return s << "▪", a;} );
    s << "◆";
    return s;
}

/* whitespace trimming for std::string */
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {
        return std::isgraph(c);
    }));
}

static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {
        return std::isgraph(c);
    }).base(), s.end());
}

static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

/* error handling */
static inline void print_error(std::string block, std::string object, std::string description) {
    std::cout << "[" << block << "] " << "'" << object << "' @ " << description << std::endl;
}

#define STRINGIFY(A) #A
#define STRINGIFYMACRO(A) STRINGIFY(A)
#define CONCATENATE(A, B) A##B
#define CONCATENATEMACRO(A, B) CONCATENATE(A, B)
#define PLACE(A, B) A B
#define EXPAND() ,,,,

#define VA_COUNT(_0, _1, _2, _3, _4, N, ...) N
#define VA_SIZE(...) PLACE(VA_COUNT, (EXPAND __VA_ARGS__ (), 0, 4, 3, 2, 1) )
#define VA_SELECT(NAME, ...) CONCATENATEMACRO(NAME, VA_SIZE(__VA_ARGS__))

#define NOP
#define CONT continue
#define RETV return
#define RETZ return 0
#define RETP return 1
#define RETN return -1
#define EXIT exit(1)

#define THROW(...) VA_SELECT(THROW, __VA_ARGS__)(__VA_ARGS__)

#define THROW0() THROW4(__FILE__, STRINGIFYMACRO(__LINE__), "warning", NOP)
#define THROW1(description) THROW4(__FILE__, STRINGIFYMACRO(__LINE__), description, NOP)
#define THROW2(description, action) THROW4(__FILE__, STRINGIFYMACRO(__LINE__), description, action)
#define THROW3(object, description, action) THROW4(__FILE__, object, description, action)
#define THROW4(block, object, description, action)                      \
    do {                                                                \
        print_error(STRINGIFYMACRO(block), object, description);        \
        action;                                                         \
    } while (0)                                                         \

#endif  /* _UTILITY_H */
