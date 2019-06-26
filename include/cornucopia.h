#ifndef _CORNUCOPIA_H
#define _CORNUCOPIA_H

#include <string>
#include <unordered_map>

/* class [cornucopia]
 * + container that holds objects of _different_ types
 * $ std::string tag required to identify objects
 */

class cornucopia {
  public:
    template<class T>
    void set(const std::string& tag, T&& value) {
        container<T>[this].emplace(tag, std::forward<T>(value)); }

    template<class T>
    void unset(const std::string& tag) {
        container<T>[this].erase(tag); }

    template<class T>
    bool test(const std::string& tag) const {
        return container<T>[this].find(tag) != container<T>[this].end(); }

    template<class T>
    const T& get(const std::string& tag) const {
        return container<T>[this][tag]; }

    template<class T>
    static decltype(auto) open(const cornucopia* obj) {
        return container<T>[obj]; }

  private:
    template<class T>
    static std::unordered_map<const cornucopia*,
        std::unordered_map<std::string, T>> container;
};

template<class T>
std::unordered_map<const cornucopia*, std::unordered_map<std::string, T>>
    cornucopia::container;

/* visitor pattern */
template<typename...>
struct type_list{};

template<typename... TYPES>
struct visitor_base { using types = type_list<TYPES...>; };

#endif  /* _CORNUCOPIA_H */
