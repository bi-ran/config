#ifndef _CONFIGURER_H
#define _CONFIGURER_H

#include <fstream>
#include <functional>
#include <list>
#include <locale>
#include <ostream>
#include <sstream>
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

        delimiter::rc = std::vector<std::ctype_base::mask>(
            std::ctype<char>::classic_table(),
            std::ctype<char>::classic_table() + std::ctype<char>::table_size);
    }

    configurer(const std::string& file) : configurer() { parse(file); }

    ~configurer() { clear(); }

    void clear();
    void parse(const std::string& file);
    void print(std::ostream& stream);

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

#define TYPE(type) type, std::vector<type>
struct create : visitor_base<REGISTRY_TYPELIST(TYPE)> {
    template<class T, typename... VS>
    void operator()(std::function<T*()>& constructor, std::tuple<VS...>& args) {
        std::stringstream& stream = std::get<2>(args);
        T* value = constructor(); stream >> (*value);

        std::get<3>(args)->set(std::get<1>(args), std::move(*value));
        delete value;
    }
};

struct destroy : visitor_base<REGISTRY_TYPELIST(TYPE)> {
    template<class T, typename... VS>
    void operator()(std::pair<const std::string, T>& value,
            std::tuple<VS...>& args) {
        std::get<0>(args)->unset(value.first, value.second);
    }
};

struct output : visitor_base<REGISTRY_TYPELIST(TYPE)> {
    template<class T, typename... VS>
    void operator()(std::pair<const std::string, T>& value,
            std::tuple<VS...>& args) {
        std::ostream& stream = std::get<0>(args);
        stream << value.first << " = " << value.second << std::endl;
    }
};

void configurer::clear() {
    visit(destroy{}, options, std::make_tuple(this));
}

void configurer::parse(const std::string& file) {
    std::ifstream fstream(file);
    if (!fstream) { error("configurer", file, "invalid file"); exit(1); }

    std::list<std::string> lines; std::string line;
    while (std::getline(fstream, line)) {
        ltrim(line); lines.emplace_back(std::move(line)); }

    for (auto l = lines.begin(); l != lines.end();) {
        auto& line = *l; ++l;
        if (line.empty() || line[0] == '#') { continue; }

        while (line.back() == '\\' && l != lines.end()) {
            line.pop_back(); line.append(*l); l = lines.erase(l); }

        std::stringstream lstream(line);
        std::string type; lstream >> type; trim(type);

        if (type == "token") {
            delimiter::reset_table(token);
            if (lstream.peek() == EOF) { token = ' '; }
            else { lstream >> token; }
            continue;
        }

        lstream.imbue(std::locale(std::locale(), new delimiter('=')));
        std::string tag; lstream >> tag; trim(tag);

        if (tag.empty()) {
            error("configurer", type, "warning: empty tag"); return; }

        delimiter::reset_table('='); lstream.ignore(1);
        lstream.imbue(std::locale(std::locale(), new delimiter(token)));

        visit(create{}, types, std::make_tuple(
            std::move(type), std::move(tag), std::ref(lstream), this));

        delimiter::reset_table(token);
    }
}

void configurer::print(std::ostream& stream = std::cout) {
    visit(output{}, options, std::make_tuple(std::ref(stream)));
}

template<class T, template<typename...> class V, typename... VS, class W>
void configurer::visit(T&& visitor, W obj, V<VS...> args) {
    visit_impl(typename std::decay<T>::type::types{}, visitor, obj, args);
}

template<class T, template<typename...> class U, typename... US,
         template<typename...> class V, typename... VS, class W>
void configurer::visit_impl(U<US...>, T&& visitor, W obj, V<VS...>& args) {
    (void)(int []) {
        0, (visit_impl_helper<T, US, VS...>(visitor, obj, args), 0)... };
}

template<class T, typename U, typename... VS>
void configurer::visit_impl_helper(T& visitor, cornucopia* obj,
                                   std::tuple<VS...>& args) {
    for (auto& element : cornucopia::container<U>[obj])
        visitor(element, args);
}

template<class T, typename U, typename... VS>
void configurer::visit_impl_helper(T& visitor, registry* obj,
                                   std::tuple<VS...>& args) {
    const auto& key = std::get<0>(args);
    auto& instance = cornucopia::container<std::function<U*()>>[obj->factory];
    if (instance.count(key)) { visitor(instance[key], args); }
}

#endif  /* _CONFIGURER_H */
