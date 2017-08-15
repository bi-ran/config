#include <vector>
#include <string>
#include <iostream>

#include "configurer.h"
#include "cornucopia.h"

#include "utility.h"

int main(int argc, char* argv[]) {
    if (argc < 2) { THROW("not enough arguments", "usage: ./example [config]", EXIT); }

    /* configurer usage */
    configurer* conf = new configurer(argv[1]);

    int a = conf->get<int>("a");
    float b = conf->get<float>("b");
    std::vector<int> c = conf->get<std::vector<int>>("c");
    std::string d = conf->get<std::string>("d");

    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    std::cout << "c: " << c << std::endl;
    std::cout << "d: " << d << std::endl;

    /* cornucopia usage */
    cornucopia* corn = new cornucopia();

    corn->set("a", a);
    corn->set("b", b);
    corn->set("c", c);
    corn->set("d", d);

    std::cout << "a: " << corn->get<int>("a") << std::endl;
    std::cout << "b: " << corn->get<float>("b") << std::endl;
    std::cout << "c: " << corn->get<std::vector<int>>("c") << std::endl;
    std::cout << "d: " << corn->get<std::string>("d") << std::endl;

    return 0;
}
