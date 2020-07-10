#include "lichtschranke.h"

Lichtschranke::Lichtschranke(int pin) : readPin(pin)
{}

void Lichtschranke::begin(){
    pinMode(0, OUTPUT);     //initialize Pin 0 as ground for tek team with GPio 2
    digitalWrite(0, LOW);   //make GPIO0 output low to be used as ground
    pinMode(readPin, INPUT);  //make reading pin an input
}

bool Lichtschranke::isTriggered(){
    return digitalRead(readPin) == HIGH;
}

bool Lichtschranke::setLichtschrankenPin(int pin){
    if(readPin != pin && pin > 0){
        readPin = pin;
        return true;
    }
    return false;
}

void Lichtschranke::setTriggerCallback(triggerCallback callback){
    m_triggerCallback = callback;
}

void Lichtschranke::handleTrigger(){
    m_now = millis();
    if(isTriggered() && m_now >= m_lastTrigger + m_timeout){
        if(m_triggerCallback)
            m_triggerCallback(m_now);
        m_lastTrigger = m_now;
    }
}

bool Lichtschranke::isNMsFromLastTrigegr(long n){
    return n + m_lastTrigger < millis();
}

void Lichtschranke::setTimeout(int timeOut){
    m_timeout = timeOut;
}