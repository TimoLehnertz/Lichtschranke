#ifndef Lichtschranke_h
#define Lichtschranke_h

#include <Arduino.h>

typedef void(*triggerCallback)(unsigned int);

class Lichtschranke{
    public:
        Lichtschranke(int = 2);
        void begin();
        void handleTrigger(); //to put in loop for polling
        bool isTriggered();
        bool setLichtschrankenPin(int);
        int readPin;
        void setTriggerCallback(triggerCallback);
        bool isNMsFromLastTrigegr(long);
        void setTimeout(int);
    private:
        triggerCallback m_triggerCallback = nullptr;
        int m_timeout = 1000;
        long m_lastTrigger = 0;
        long m_now = 0;
};

#endif