// account6.cpp

#include <concepts>
#include <iostream>

template<typename T>
concept Equal =
    requires(T a, T b) {
        { a == b } -> std::convertible_to<bool>;
        { a != b } -> std::convertible_to<bool>;
    };


template <typename T>
concept Ordering =
    Equal<T> &&
    requires(T a, T b) {
        { a <= b } -> std::convertible_to<bool>;
        { a < b } -> std::convertible_to<bool>;
        { a > b } -> std::convertible_to<bool>;
        { a >= b } -> std::convertible_to<bool>;
    };

class Account {
 public:
    Account() = default;
    Account(double bal): balance{bal} {}
    auto operator <=> (const Account& oth) const = default;
 private:
    double balance{0.0};
};

template <Ordering T>
bool isSmaller(T t, T t2) {
    return t < t2;
}

template <Ordering T>
bool isGreater(T t, T t2) {
    return t > t2;
}

int main() {

    std::cout << std::boolalpha;

    double doub1{};
    double doub2{10.5};
    std::cout << "isSmaller(doub1, doub2): " << isSmaller(doub1, doub2) << '\n';
    std::cout << "isGreater(doub1, doub2): " << isGreater(doub1, doub2) << '\n';
    
    Account acc1;
    Account acc2(10.5);
    std::cout << "isSmaller(acc1, acc2): " << isSmaller(acc1, acc2) << '\n';
    std::cout << "isGreater(acc1, acc2): " << isGreater(acc1, acc2) << '\n';

}