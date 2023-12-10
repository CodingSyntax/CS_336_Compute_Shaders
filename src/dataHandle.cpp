#include "dataHandle.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>

void Data::setList() {    
    list = (float*)calloc(args * actualSize, sizeof(float));
}

Data::Data(int args) : args(args), size(0), actualSize(8) {
    setList();
}

Data::~Data() {
    free(list);
}

//Be warned of garbage data possible at the end of allocated memory
float *Data::getData() {
    return list;
}

unsigned int Data::getSize() {
    return size;
}

unsigned int Data::getFullSize() {
    return sizeof(float) * args * size;
}

int Data::checkAdd() {
    size++;
    if (actualSize < size) {
        float *oldList = list;
        unsigned int oldSize = actualSize;
        actualSize *= 2;
        setList();
        memcpy(list, oldList, oldSize * sizeof(float) * args);
        std::cout << args << std::endl;
        free(oldList);
    }
    return (size - 1) * args;
}

float *Data::operator[](unsigned int index) {
    if (index >= size) throw "Index out of bounds";
    return list + (index * args);
}




Data3::Data3() : Data(3) {}

Data3::~Data3() {}

void Data3::add(float f[3]) {
    int ind = checkAdd();
    *(list + ind) = f[0];
    *(list + ind + 1) = f[1];
    *(list + ind + 2) = f[2];
    
    // std::cout << "{";
    // for (int i = 0; i < size; i++) {
    //     int ind = i * 3;
    //     std::cout << "{" << list[ind] << "," << list[ind + 1] << "," << list[ind + 2] << "}" << std::endl;
    // }
    // std::cout << "}" << std::endl;
}

Data2::Data2() : Data(2) {}

Data2::~Data2() {}

void Data2::add(float f[2]) {
    int ind = checkAdd();
    *(list + ind) = f[0];
    *(list + ind + 1) = f[1];
}

void Data2::zero() {
    // std::cout << "ptr: " << list << std::endl;
    
    // std::cout << "{";
    // for (int i = 0; i < size; i++) {
    //     int ind = i * 2;
    //     std::cout << "{" << list[ind] << "," << list[ind + 1] << "}" << std::endl;
    // }
    // std::cout << "}" << std::endl;
    // std::cout << size << std::endl;
    for (float *i = list; i < (list + size * 2); i++) {
        // std::cout << *i << std::endl;
        *i = 0;
    }
    // free(list);
    // setList();
}

Data5::Data5() : Data(5) {}

Data5::~Data5() {}

void Data5::add(float f[5]) {
    int ind = checkAdd();
    *(list + ind) = f[0];
    *(list + ind + 1) = f[1];
    *(list + ind + 2) = f[2];
    *(list + ind + 3) = f[3];
    *(list + ind + 4) = f[4];
    
    // std::cout << "{";
    // for (int i = 0; i < size; i++) {
    //     int ind = i * 5;
    //     std::cout << "{" << list[ind] << "," << list[ind + 1] << "," << list[ind + 2] << "," << list[ind + 3] << "," << list[ind + 4] << "}" << std::endl;
    // }
    // std::cout << "}" << std::endl;
}