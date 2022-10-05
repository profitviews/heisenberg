// account2.cpp

#include <concepts>
#include <iostream>

template <typename T>
concept Smaller = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
};

class Account {
 public:
    Account() = default;
    Account(double bal): balance{bal} {}
 private:
    double balance{0.0};
};

template <Smaller T>
bool isSmaller(T t, T t2) {
    return t < t2;
}

int main() {

    std::cout << std::boolalpha;
    
    double doub1{};
    double doub2{10.5};
    std::cout << "isSmaller(doub1, doub2): " << isSmaller(doub1, doub2) << '\n';

    Account acc1;
    Account acc2(10.5);
    std::cout << "isSmaller(acc1, acc2): " << isSmaller(acc1, acc2) << '\n';

}