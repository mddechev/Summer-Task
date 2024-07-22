#include "DataSource.hpp"
#include <exception>
#include <ios>
#include <iostream>

int main() {
    try {
        int array[] = {1, 2, 3, 4, 5, 6};
        size_t size = 6;

        // DataSource<int>* source = new ArrayDataSource<int>(array, size);
        ArrayDataSource<int> src(array, size);

        // std::cout << source->getElement() << ' ' << source->operator()() << '\n';
        // for (size_t i = 0; i < size; i++) {
        //     std::cout << "element" << i + 1 << ": " << source->getElement() << '\n';
        // }

        // if (!source->hasNext()) {
        //     std::cout << "No more data! Please reset\n";
        //     if (source->reset()) {
        //         std::cout << "Reset successfull!\n";
        //     } else {
        //         std::cout << "Unsuccessfull reset!\n";
        //     }

        // } else {
        //     std::cout << source->operator()() << '\n';
        // }

        // int* batch = source->getElementsInBatch(10);
        // for (size_t i = 0; i < 6; i++) {
        //     std::cout << batch[i] << ' ';
        // }
        // std::cout << std::boolalpha << source->hasNext() << '\n';
        // std::cout << std::boolalpha << source->operator bool() << '\n';
        // if (source->operator bool()) {
        //     std::cout << source->getElement() << '\n';
        // } else {
        //     std::cout << "No more data! Reset\n";
        //     if (source->reset()) {
        //         std::cout << "reset successful!\n";
        //     }
        // }
        int a = 25;
        int b = 36;
        std::cout << a << ' ' << b << '\n';
        src >> a >> b;
        std::cout << a << ' ' << b << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}