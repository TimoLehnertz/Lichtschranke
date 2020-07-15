#pragma once

class SerialInterface{
    public:
        virtual bool available() = 0;
        virtual char read() = 0;
        virtual void write(const char*) = 0;
};