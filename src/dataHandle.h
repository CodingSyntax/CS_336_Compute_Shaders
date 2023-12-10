#pragma once

class Data {
    protected:
        unsigned int args;
        unsigned int size;
        unsigned int actualSize;
        float *list;
        
        void setList();
        int checkAdd();
    public:
        Data(int args);
        virtual ~Data();
        
        unsigned int getSize();
        unsigned int getFullSize();
        float *getData();
        float *operator[](unsigned int index);
};

class Data3 : public Data {
    public:
        Data3();
        ~Data3();
        
        void add(float f[3]);
};

class Data2 : public Data {
    public:
        Data2();
        ~Data2();
        
        void add(float f[2]);
        void zero();
};

class Data5 : public Data {
    public:
        Data5();
        ~Data5();
        
        void add(float f[5]);
};