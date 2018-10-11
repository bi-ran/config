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

struct delimiter : std::ctype<char> {
    delimiter(char token) : std::ctype<char>(get_table(token), false, 0) {}

    static std::vector<std::ctype_base::mask> rc;

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
};

std::vector<std::ctype_base::mask> delimiter::rc;

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

    void parse(const std::string& file);
    void print(std::ostream& stream);

    template<class T>
    void set(const std::string& tag, T&& value) {
        options->set(tag, std::forward<T>(value)); }

    template<class T>
    void unset(const std::string& tag) {
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
        std::stringstream& stream = std::get<1>(args);

        stream.imbue(std::locale(std::locale(), new delimiter('=')));
        std::string tag; stream >> tag; trim(tag);

        if (tag.empty())
            THROW(configurer, std::get<0>(args), "warning: empty tag", RETV);
        for (char& c : tag) {
            if (!std::isgraph(c))
                THROW(configurer, tag, "invalid char in tag", EXIT);
        }

        delimiter::reset_table('=');
        stream.ignore(1);

        char token = std::get<2>(args);
        stream.imbue(std::locale(std::locale(), new delimiter(token)));
        T* value = constructor(); stream >> (*value);

        delimiter::reset_table(token);
        if (stream.bad()) { THROW(configurer, tag, "read error", EXIT); }

        std::get<3>(args)->set(tag, std::move(*value));
        delete value;
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

void configurer::parse(const std::string& file) {
    std::ifstream fstream(file);
    if (!fstream) { THROW(configurer, file, "invalid file", EXIT); }

    std::list<std::string> lines; std::string line;
    while (std::getline(fstream, line)) {
        ltrim(line); lines.emplace_back(std::move(line)); }

    for (auto l = lines.begin(); l != lines.end();) {
        auto& line = *l; ++l;
        if (line.empty() || line[0] == '#') { continue; }

        while (line.back() == '\\' && l != lines.end()) {
            line.pop_back(); line.append(*l); l = lines.erase(l); }

        std::stringstream lstream(line);
        std::string type; lstream >> type;

        trim(type);
        if (type == "token") {
            delimiter::reset_table(token);
            if (lstream.peek() == EOF) { token = ' '; }
            else { lstream >> token; }
            continue;
        }

        visit(create{}, types, std::make_tuple(
            std::move(type), std::ref(lstream), token, this));
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
    for (auto& element : cornucopia::container<
            std::function<U*()>>[obj->factory])
        if (element.first == std::get<0>(args))
            visitor(element.second, args);
}

#endif  /* _CONFIGURER_H */
