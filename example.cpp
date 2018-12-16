#include <iostream>
#include <string>
#include <vector>

#include "include/configurer.h"

int main(int argc, char* argv[]) {
    if (argc != 2) { error(argv[0], "", "invalid arguments"); }

    /* configurer usage */
    configurer* conf = new configurer(argv[1]);

    auto a = conf->get<int>("a");
    auto b = conf->get<float>("b");
    auto c = conf->get<std::string>("c");
    auto d = conf->get<std::vector<int>>("d");
    auto e = conf->get<std::vector<std::string>>("e");
    auto f = conf->get<std::vector<std::string>>("f");
    auto g = conf->get<std::vector<std::string>>("g");

    /* mark a, b as unused */
    (void)a; (void)b;

    conf->print();

    return 0;
}
