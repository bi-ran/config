#ifndef _CONFIGURER_H
#define _CONFIGURER_H

#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

#include "registry.h"
#include "cornucopia.h"

#include "utility.h"

struct delimiter : std::ctype<char> {
    delimiter(char token) : std::ctype<char>(get_table(token), false, 0) {}

    static std::vector<std::ctype_base::mask> rc;

    static const std::ctype_base::mask* get_table(char token) {
        set_table(token);
        return &rc[0];
    }

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
            std::ctype<char>::classic_table() + std::ctype<char>::table_size
        );
    }

    configurer(std::string file) : configurer() { parse(file); }

    void load(std::string file) { parse(file); }

    template<class T>
    void set(std::string tag, T value) { options->set(tag, value); }

    template<class T>
    void unset(std::string tag) { options->unset<T>(tag); }

    template<class T>
    bool test(std::string tag) { return options->test<T>(tag); }

    template<class T>
    T get(std::string tag) { return options->get<T>(tag); }

    char delimiter() { return token; }

  protected:
    void parse(std::string file);

    template<class T>
    void visit(T&& visitor, std::string identifier, std::iostream& stream);

    template<class T, template<class...> class TLIST, class... TYPES>
    void visit_impl(T&& visitor, std::string identifier, std::iostream& stream,
                    TLIST<TYPES...>);

    template<class T, class U>
    void visit_impl_helper(T& visitor, std::string identifier,
                           std::iostream& stream);

  private:
    registry* types;
    cornucopia* options;

    char token;
};

#define TYPE(type) type, std::vector<type>
struct create : visitor_base<REGISTRY_TYPELIST(TYPE)> {
    template<class T>
    void operator()(std::function<T*()> constructor, std::iostream& stream,
                    configurer* config) {
        stream.imbue(std::locale(std::locale(), new delimiter('=')));
        std::string tag; stream >> tag;

        trim(tag);
        if (tag.empty()) { THROW(configurer, tag, "warning: empty tag", RETV); }
        for (char& c : tag) {
            if (!std::isgraph(c))
                THROW(configurer, tag, "invalid char in tag", EXIT);
        }

        delimiter::reset_table('=');
        stream.ignore(1);

        stream.imbue(std::locale(std::locale(),
                     new delimiter(config->delimiter())));
        T* value = constructor(); stream >> (*value);

        delimiter::reset_table(config->delimiter());
        if (stream.bad()) { THROW(configurer, tag, "read error", EXIT); }

        config->set(tag, *value);
    }
};

void configurer::parse(std::string file) {
    std::ifstream file_stream(file);
    if (!file_stream) { THROW(configurer, file, "invalid file", EXIT); }

    std::vector<std::string> lines;

    std::string line;
    while (std::getline(file_stream, line)) {
        ltrim(line); lines.push_back(line);
    }

    for (std::size_t i=0; i<lines.size(); ++i) {
        if (lines[i].empty() || lines[i][0] == '#') { continue; }

        while (lines[i].back() == '\\' && i != lines.size()) {
            lines[i].pop_back(); lines[i].append(lines[i+1]);
            lines.erase(lines.begin()+i+1);
        }

        std::stringstream line_stream(lines[i]);
        std::string identifier; line_stream >> identifier;

        trim(identifier);
        if (identifier == "token") {
            delimiter::reset_table(token);
            if (line_stream.peek() == EOF) { token = ' '; }
            else { line_stream >> token; }
            continue;
        }

        visit(create{}, identifier, line_stream);
    }
}

template<class T>
void configurer::visit(T&& visitor, std::string identifier,
                       std::iostream& stream) {
    visit_impl(visitor, identifier, stream, typename std::decay_t<T>::types{});
}

template<class T, template<class...> class TLIST, class... TYPES>
void configurer::visit_impl(T&& visitor, std::string identifier,
                            std::iostream& stream, TLIST<TYPES...>) {
    (void)(int []) {
        0, (visit_impl_helper<T, TYPES>(visitor, identifier, stream), 0)...
    };
    /* C++ 17 feature required */
    /* (..., visit_impl_helper<std::decay_t<T>, TYPES>(visitor, identifier, stream)); */
}

template<class T, class U>
void configurer::visit_impl_helper(T& visitor, std::string identifier,
                                   std::iostream& stream) {
    for (std::pair< std::string, std::function<U*()> > element :
            cornucopia::container< std::function<U*()> >[types->factory]) {
        if (!identifier.empty() && identifier != element.first) { continue; }

        visitor(element.second, stream, this);
    }
}

#endif  /* _CONFIGURER_H */
