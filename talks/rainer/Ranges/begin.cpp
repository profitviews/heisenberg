// begin.cpp

#include <cstddef>
#include <iostream>
#include <ranges>

struct ContainerFree {
    ContainerFree(std::size_t len): len_(len), data_(new int[len]){}
    size_t len_;
    int* data_;
};
int* begin(const ContainerFree& conFree) {
    return conFree.data_;
}

struct ContainerMember {
    ContainerMember(std::size_t len): len_(len), data_(new int[len]){}
    int* begin() const {
        return data_;
    }
    size_t len_;
    int* data_;
};

void callBeginFree(const auto& cont) {
    begin(cont);
}

void callBeginMember(const auto& cont) {
    cont.begin();
}
 
int main() {

    const ContainerFree contFree(2020);
    const ContainerMember contMemb(2023);

    callBeginFree(contFree);
    callBeginMember(contMemb);

    callBeginFree(contMemb);
    callBeginMember(contFree);
   
}