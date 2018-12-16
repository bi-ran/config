#ifndef _CONFIGURER_H
#define _CONFIGURER_H

#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "cornucopia.h"
#include "registry.h"
#include "utility.h"

class configurer {
  public:
    configurer() {
        types = new registry();
        options = new cornucopia();

        token = ' ';
        delimiter::init_table();
    }

    configurer(const std::string& file) : configurer() { parse(file); }

    ~configurer() { clear(); }

    void clear();
    void parse(const std::string& file);
    void print(std::ostream& stream = std::cout);

    template<class T>
    void set(const std::string& tag, T&& value) {
        options->set(tag, std::forward<T>(value)); }

    template<class T>
    void unset(const std::string& tag) {
        options->unset<T>(tag); }

    template<class T>
    void unset(const std::string& tag, T&) {
        options->unset<T>(tag); }

    template<class T>
    bool test(const std::string& tag) const {
        return options->test<T>(tag); }

    template<class T>
    const T& get(const std::string& tag) const {
        return options->get<T>(tag); }

  protected:
    template<class T, template<typename...> class V, typename... VS, class W>
    void visit(T&& visitor, W obj, V<VS...> args);

    template<class T, template<typename...> class U, typename... US,
             template<typename...> class V, typename... VS, class W>
    void visit_impl(U<US...>, T&& visitor, W obj, V<VS...>& args);

    template<class T, typename U, typename... VS>
    void visit_impl_helper(T& visitor, cornucopia* obj, std::tuple<VS...>& args);

    template<class T, typename U, typename... VS>
    void visit_impl_helper(T& visitor, registry* obj, std::tuple<VS...>& args);

  private:
    registry* types;
    cornucopia* options;

    char token;
};


#endif  /* _CONFIGURER_H */
