#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <type_traits>
#include <iostream>
#include <functional>
#include <vector>
#include <string>

#include "cornucopia.h"

#include "utility.h"

/* SFINAE to check correctness of correct operator>> overload */
template<typename ...> using void_t = void;

template<typename T, typename = void>
struct has_extraction_operator : std::false_type {};

template<typename T>
struct has_extraction_operator<T, void_t<decltype(std::declval<std::istream&>() >> std::declval<T&>())>> : std::true_type {};

template<typename T, typename = void>
struct has_correct_return_type : std::false_type {};

template<typename T>
struct has_correct_return_type<T, typename std::enable_if<std::is_convertible<decltype(std::declval<std::istream&>() >> std::declval<T&>()), std::istream&>::value>::type> : std::true_type {};

/* operator>> overload for std::vector<T> */
template<class T>
std::istream& operator>>(std::istream& s, std::vector<T>& v) {
    std::copy(std::istream_iterator<T>(s), std::istream_iterator<T>(),
              std::back_inserter(v));
    return s;
}

template<class T>
T* constructor() { return new T(); }

/* class [registry]
 * + resolves identifier string into type, and constructs object
 * $ additional types must satisfy the following conditions:
 *   - default constructor is defined
 *   - istream& operator>>(type& val) is implemented
 */

#define REGISTRY_TYPELIST(ELEMENT)  \
    ELEMENT(bool),                  \
    ELEMENT(char),                  \
    ELEMENT(short),                 \
    ELEMENT(int),                   \
    ELEMENT(long),                  \
    ELEMENT(uint8_t),               \
    ELEMENT(uint16_t),              \
    ELEMENT(uint32_t),              \
    ELEMENT(uint64_t),              \
    ELEMENT(float),                 \
    ELEMENT(double),                \
    ELEMENT(std::string)            \

#define REGISTER_TYPE(identifier)                                           \
    register_type<identifier>(#identifier),                                 \
    register_type<std::vector<identifier>>("std::vector<"#identifier">")    \

class registry {
  public:
    registry() { REGISTRY_TYPELIST(REGISTER_TYPE); }

    template<class T>
    void register_type(const std::string& identifier) {
        factory->set(identifier, std::function<T*()>(&constructor<T>));
    }

    template<class T>
    T* construct(std::string& identifier) {
        if (!factory->test<T>(identifier))
            THROW(registry, identifier, "type not supported", EXIT);

        return factory->get<T>(identifier)();
    }

    cornucopia* factory;
};

#endif  /* _REGISTRY_H */
