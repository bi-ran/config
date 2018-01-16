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
    std::string c = conf->get<std::string>("c");
    std::vector<int> d = conf->get<std::vector<int>>("d");
    std::vector<std::string> e = conf->get<std::vector<std::string>>("e");
    std::vector<std::string> f = conf->get<std::vector<std::string>>("f");
    std::vector<std::string> g = conf->get<std::vector<std::string>>("g");

    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    std::cout << "c: " << c << std::endl;
    std::cout << "d: " << d << std::endl;
    std::cout << "e: " << e << std::endl;
    std::cout << "f: " << f << std::endl;
    std::cout << "g: " << g << std::endl;

    return 0;
}
