#ifndef _UTILITY_H
#define _UTILITY_H

#include <string>
#include <algorithm>
#include <iostream>

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

/* operator<< overload for std::vector<T> */
template<class T>
std::ostream& operator<<(std::ostream& s, std::vector<T> v) {
    s << "[ ";
    std::copy(begin(v), end(v), std::ostream_iterator<T>(s, " "));
    s << "]";
    return s;
}

/* error handling */
static inline void throw_error(std::string name, std::string object, std::string description) {
    std::cout << "[" << name << "] '" << object << "': " << description << std::endl;
}

#define STRINGIFY(A) #A
#define CONCATENATE(A, B) A##B
#define CONCATENATEMACRO(A, B) CONCATENATE(A, B)

#define VA_COUNT(_1, _2, _3, N, ...) N
#define VA_SIZE(...) VA_COUNT(__VA_ARGS__, 3, 2, 1)
#define VA_SELECT(NAME, ...) CONCATENATEMACRO(NAME, VA_SIZE(__VA_ARGS__))

#define FATAL0 return
#define FATAL1 exit(1)
#define FATAL(bool) FATAL##bool

#define THROW(...) VA_SELECT(THROW, __VA_ARGS__)(__VA_ARGS__)

#define THROW1(description) THROW3(, description, 0)
#define THROW2(object, description) THROW3(object, description, 0)
#define THROW3(object, description, fatal)      \
    throw_error(BLOCK, object, description);    \
    FATAL(fatal);                               \

#endif  /* _UTILITY_H */
