#ifndef _CORNUCOPIA_H
#define _CORNUCOPIA_H

#include <unordered_map>
#include <string>

#include <vector>
#include <iostream>

/* class [cornucopia]
 * + container that can hold objects of _different_ types simultaneously
 * $ std::string tag required to identify objects
 */

#define BLOCK cornucopia
class cornucopia {
  public:
    template<class T>
    void set(std::string tag, const T& value) {
        container<T>[this][tag] = value;
    }

    template<class T>
    void unset(std::string tag) {
        container<T>[this].erase(tag);
    }

    template<class T>
    bool test(std::string tag) {
        return container<T>[this].find(tag) != container<T>[this].end();
    }

    template<class T>
    T get(std::string tag) {
        return container<T>[this][tag];
    }

    template<class T>
    static std::unordered_map< const cornucopia*, std::unordered_map<std::string, T> > container;
};
#undef BLOCK

template<class T>
std::unordered_map< const cornucopia*, std::unordered_map<std::string, T> > cornucopia::container;

/* visitor pattern */
template<class...>
struct type_list{};

template<class... TYPES>
struct visitor_base { using types = type_list<TYPES...>; };

#endif  /* _CORNUCOPIA_H */
