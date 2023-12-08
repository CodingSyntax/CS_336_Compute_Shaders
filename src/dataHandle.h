#pragma once

class Data3 {
    private:
        unsigned int size;
        unsigned int actualSize;
        float *list;
        
        void setList();
    public:
        Data3();
        ~Data3();
        
        unsigned int getSize();
        unsigned int getFullSize();
        float *getData();
        void add(float f[3]);
        float *operator[](unsigned int index);
};

class Data2 {
    private:
        unsigned int size;
        unsigned int actualSize;
        float *list;
        
        void setList();
    public:
        Data2();
        ~Data2();
        
        unsigned int getSize();
        unsigned int getFullSize();
        float *getData();
        void add(float f[2]);
        void zero();
        
        float *operator[](unsigned int index);
};