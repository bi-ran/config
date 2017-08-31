#ifndef _CONFIGURER_H
#define _CONFIGURER_H

#include <string>
#include <vector>
#include <iterator>
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

#define BLOCK configurer
class configurer {
  public:
    configurer() {
        types = new registry();
        options = new cornucopia();
        token = ' ';

        delimiter::rc = std::vector<std::ctype_base::mask>(std::ctype<char>::classic_table(), std::ctype<char>::classic_table() + std::ctype<char>::table_size);
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

  protected:
    void parse(std::string file);

    template<class T>
    void visit(T&& visitor, std::string identifier, std::stringstream& line_stream, std::string tag);

    template<class T, template<class...> class TLIST, class... TYPES>
    void visit_impl(T&& visitor, std::string identifier, std::stringstream& line_stream, std::string tag, TLIST<TYPES...>);

    template<class T, class U>
    void visit_impl_helper(T& visitor, std::string identifier, std::stringstream& line_stream, std::string tag);

  private:
    registry* types;
    cornucopia* options;

    char token;
};

#define ELEMENT(type) type, std::vector<type>
struct visitor : visitor_base<REGISTRY_TYPELIST> {
    template<class T>
    void operator()(std::function<T*()> constructor, std::stringstream& line_stream, std::string tag, configurer* config) {
        T* value = constructor();
        line_stream >> (*value);

        if (line_stream.bad()) { THROW(tag, "read error", RETV); }

        config->set(tag, *value);
    }
};
#undef ELEMENT

void configurer::parse(std::string file) {
    std::ifstream file_stream(file);
    if (!file_stream) { THROW(file, "invalid file", EXIT); }

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file_stream, line))
        lines.push_back(line);

    for (std::size_t i=0; i<lines.size(); ++i) {
        ltrim(lines[i]);
        if (lines[i].empty() || lines[i][0] == '#')
            continue;

        std::stringstream line_stream(lines[i]);

        std::string identifier;
        line_stream >> identifier;

        trim(identifier);
        if (identifier == "token") {
            delimiter::reset_table(token);
            if (line_stream.peek() == EOF) { token = ' '; }
            else { line_stream >> token; }
            continue;
        }

        line_stream.imbue(std::locale(std::locale(), new delimiter('=')));
        std::string tag;
        line_stream >> tag;
        delimiter::reset_table('=');

        line_stream.ignore(1);

        trim(tag);
        if (tag.empty()) { continue; }
        for (char& c : tag) {
            if (!std::isgraph(c)) {
                THROW(tag, "invalid char in tag", EXIT);
            }
        }

        line_stream.imbue(std::locale(std::locale(), new delimiter(token)));
        visit(visitor{}, identifier, line_stream, tag);
        delimiter::reset_table(token);
    }
}

template<class T>
void configurer::visit(T&& visitor, std::string identifier, std::stringstream& line_stream, std::string tag) {
    visit_impl(visitor, identifier, line_stream, tag, typename std::decay_t<T>::types{});
}

/* C++ 17 feature required */
template<class T, template<class...> class TLIST, class... TYPES>
void configurer::visit_impl(T&& visitor, std::string identifier, std::stringstream& line_stream, std::string tag, TLIST<TYPES...>) {
    (..., visit_impl_helper<std::decay_t<T>, TYPES>(visitor, identifier, line_stream, tag));
}

template<class T, class U>
void configurer::visit_impl_helper(T& visitor, std::string identifier, std::stringstream& line_stream, std::string tag) {
    for (std::pair< std::string, std::function<U*()> > element : cornucopia::container< std::function<U*()> >[types->factory]) {
        if (element.first == identifier)
            visitor(element.second, line_stream, tag, this);
    }
}
#undef BLOCK

#endif  /* _CONFIGURER_H */
