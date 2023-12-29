#ifndef BMP_H
#define BMP_H

#include <cstdint>

class Bitmap{
private:
    uint8_t size;
    int* array;
public:
    void init(int size);
    void set(int index);
    void reset(int index);
    bool test(int index);
    void flip(int index);

    ~Bitmap();
};

#endif // BMP_H

