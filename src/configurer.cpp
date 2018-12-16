#include <fstream>
#include <functional>
#include <list>
#include <locale>
#include <sstream>

#include "../include/configurer.h"

std::vector<std::ctype_base::mask> delimiter::rc;

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
    for (auto& element : cornucopia::open<U>(obj))
        visitor(element, args);
}

template<class T, typename U, typename... VS>
void configurer::visit_impl_helper(T& visitor, registry* obj,
                                   std::tuple<VS...>& args) {
    const auto& key = std::get<0>(args);
    auto& instance = cornucopia::open<std::function<U*()>>(obj->factory);
    if (instance.count(key)) { visitor(instance[key], args); }
}

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

void configurer::clear() {
    visit(destroy{}, options, std::make_tuple(this));
}

void configurer::print(std::ostream& stream) {
    visit(output{}, options, std::make_tuple(std::ref(stream)));
}
