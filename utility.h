#ifndef _UTILITY_H
#define _UTILITY_H

#include <string>
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
static inline void print_error(std::string name, std::string error, std::string description) {
    std::cout << "[" << name << "] '" << error << "': " << description << std::endl;
}

#define STRINGIFY(A) #A
#define STRINGIFYMACRO(A) STRINGIFY(A)
#define CONCATENATE(A, B) A##B
#define CONCATENATEMACRO(A, B) CONCATENATE(A, B)
#define PLACE(A, B) A B
#define EXPAND() ,,,

#define VA_COUNT(_0, _1, _2, _3, N, ...) N
#define VA_SIZE(...) PLACE(VA_COUNT, (EXPAND __VA_ARGS__ (), 0, 3, 2, 1) )
#define VA_SELECT(NAME, ...) CONCATENATEMACRO(NAME, VA_SIZE(__VA_ARGS__))

#define FATALNOP
#define FATALCONT continue
#define FATALRETV return
#define FATALRETI return 1
#define FATALEXIT exit(1)
#define FATAL(LEVEL) CONCATENATEMACRO(FATAL, LEVEL)

#define THROW(...) VA_SELECT(THROW, __VA_ARGS__)(__VA_ARGS__)

#define THROW0() THROW3(, "warning", 0)
#define THROW1(description) THROW3(, description, 0)
#define THROW2(error, description) THROW3(error, description, 0)
#define THROW3(error, description, fatal)                       \
    do {                                                        \
        print_error(STRINGIFYMACRO(BLOCK), error, description); \
        FATAL(fatal);                                           \
    } while (0)                                                 \

#endif  /* _UTILITY_H */
