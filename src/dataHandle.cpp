#include "dataHandle.h"
#include <stdlib.h>
#include <cstring>

void Data3::setList() {
    list = (float*)malloc(sizeof(float) * 3 * actualSize);
}

Data3::Data3() : size(0), actualSize(8) {
    setList();
}

Data3::~Data3() {
    free(list);
}

//Be warned of garbage data possible at the end of allocated memory
float *Data3::getData() {
    return list;
}

unsigned int Data3::getSize() {
    return size;
}

unsigned int Data3::getFullSize() {
    return sizeof(float) * 3 * size;
}

void Data3::add(float f[3]) {
    size++;
    if (actualSize < size) {
        float *oldList = list;
        unsigned int oldSize = actualSize;
        actualSize *= 2;
        setList();
        memcpy(list, oldList, oldSize);
        free(oldList);
    }
    *(list + size) = f[0];
    *(list + size + 1) = f[1];
    *(list + size + 2) = f[2];
}

float *Data3::operator[](unsigned int index) {
    if (index >= size) throw "Index out of bounds";
    return list + (index * 3);
}





void Data2::setList() {
    list = (float*)malloc(sizeof(float) * 2 * actualSize);
}

Data2::Data2() : size(0), actualSize(8) {
    setList();
}

Data2::~Data2() {
    free(list);
}

//Be warned of garbage data possible at the end of allocated memory
float *Data2::getData() {
    return list;
}

unsigned int Data2::getSize() {
    return size;
}

unsigned int Data2::getFullSize() {
    return sizeof(float) * 2 * size;
}

void Data2::add(float f[2]) {
    size++;
    if (actualSize < size) {
        float *oldList = list;
        unsigned int oldSize = actualSize;
        actualSize *= 2;
        setList();
        memcpy(list, oldList, oldSize);
        free(oldList);
    }
    *(list + size) = f[0];
    *(list + size + 1) = f[1];
}

float *Data2::operator[](unsigned int index) {
    if (index >= size) throw "Index out of bounds";
    return list + (index * 2);
}