#include <Arduino.h>

class HardwareInterface{
    virtual uint64_t getNanos() = 0;
    virtual uint64_t getMicros() = 0;
    virtual uint64_t getMillis() = 0;
    virtual uint32_t getSeconds() = 0;

    virtual void begin() = 0;

    virtual char read() = 0;
    virtual bool available() = 0;
    virtual void write(char) = 0;
};