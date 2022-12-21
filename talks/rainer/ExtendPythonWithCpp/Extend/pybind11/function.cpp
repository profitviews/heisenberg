#include <pybind11/pybind11.h>

namespace py  = pybind11;

int add(int i, int j) {
    return i + j;
}

int sum(int i, int j) { return add(i, j); }
int sum(int i, int j, int k) { return i + j + k;}

PYBIND11_MODULE(function, m) {
    m.def("add", &add, "A function which adds two numbers",
        py::arg("i") = 2000, py::arg("j") = 11);
    m.def("sum", py::overload_cast<int, int>(&sum), "Sum up two values");
    m.def("sum", py::overload_cast<int, int, int>(&sum), "Sum up three values");

    m.attr("year") = 2011;
    m.attr("language") = "C++11";
}

// g++ -O3 -Wall -shared -std=c++14 -fPIC $(python3.6 -m pybind11 --includes) function.cpp -o function$(python3.6-config --extension-suffix)
