#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "forward.h"

#include "cornucopia.h"
#include "utility.h"

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
    ELEMENT(int8_t),                \
    ELEMENT(int16_t),               \
    ELEMENT(int32_t),               \
    ELEMENT(int64_t),               \
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
    friend class configurer;

  public:
    registry() { REGISTRY_TYPELIST(REGISTER_TYPE); }

    template<class T>
    void register_type(const std::string& identifier) {
        factory->set(identifier, std::function<T*()>(&constructor<T>)); }

    template<class T>
    T* construct(const std::string& identifier) const {
        return factory->get<T>(identifier)(); }

  private:
    cornucopia* factory;
};

#endif  /* _REGISTRY_H */
