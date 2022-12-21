#include <pybind11/pybind11.h>
#include <string>

struct HumanBeing {
    HumanBeing(const std::string& n) : name(n) { }
    const std::string& getName() const { return name; }
    std::string name;
    std::string familyName{"Grimm"};
    virtual ~HumanBeing() = default;
};

struct Man: HumanBeing {
    Man(const std::string& name): HumanBeing(name) { }
    std::string gender() const { return "male"; }
};

namespace py = pybind11;

PYBIND11_MODULE(human, m) {
    py::class_<HumanBeing>(m, "HumanBeing")
        .def(py::init<const std::string &>())
        .def("getName", &HumanBeing::getName)
        .def("__repr__", [](const HumanBeing& h) { return "HumanBeing: " + h.name; })
        .def_readwrite("familyName", &HumanBeing::familyName);

    py::class_<Man, HumanBeing>(m, "Man")
        .def(py::init<const std::string &>())
        .def("gender", &Man::gender);
        
}

// g++ -O3 -Wall -shared -std=c++14 -fPIC $(python3.6 -m pybind11 --includes) human.cpp -o human$(python3.6-config --extension-suffix)