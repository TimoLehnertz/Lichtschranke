#include "SerialCom.h"

#define STACK false

//Structure of com: 1 byte start cap, 1 byte port, 1 byte type, 1 byte statusCode, 1 byte requestID, 1 - 249 bytes message(must be null terminated), 1 byte end cap
//Types:
//Broadcast(No response)
//Request(sends response to port of requestID)
//Response(response to request)

SerialCom::SerialCom() : m_comSize(0), m_listenerSize(0), m_startCap(-1), m_debug(nullptr), m_sendQueSize(0){
    for (size_t i = 0; i < 255; i++)
    {
        m_broadcastListeners[i] = nullptr;
        m_requestListeners[i] = nullptr;
        m_responseCallbacks[i] = nullptr;
    }
}

void SerialCom::begin(){

}

void SerialCom::setDebug(SoftwareSerial& debug){
    m_debug = &debug;
}

void SerialCom::setDebug(void * ptr){
    if(!ptr)
        m_debug = nullptr;
}

void SerialCom::handleComunication(){
    if(STACK)
        m_debug->println("handleComunication begin");
    while(available()){
        m_buffer = read();
        if(m_buffer == delimiter){
            m_startCap = m_comSize;
            m_comBuffer[m_comSize] = m_buffer;
            m_comSize++;
            processCombuffer();
        } else{
            m_comBuffer[m_comSize] = m_buffer;
            m_comSize++;
        }
    }
    executeTimeoutResponseCallbacks();
    if(STACK)
        m_debug->println("handleComunication end");
}

void SerialCom::processCombuffer(){
    if(STACK)
        m_debug->println("processCombuffer begin");
    if(isComBufferValid()){
        Com com(m_comBuffer, m_startCap, m_comSize);
        if(com.type == BROADCAST){
            processBroadcast(com);
        } else if(com.type == REQUEST){
            processRequest(com);
        } else if(com.type == RESPONSE){
            if(m_responseCallbacks[com.requestID]){
                (*m_responseCallbacks[com.requestID])(com);
                removeResponseCallback(com.requestID);
            } else if(m_debug){
                m_debug->println("no matching response callback found for id " + String(com.requestID));
            }
        }
    }
    m_startCap = -1;
    m_comSize = 0;
    if(STACK)
        m_debug->println("processCombuffer end");
}

void SerialCom::executeTimeoutResponseCallbacks(){
    if(STACK)
        m_debug->println("executeTimeoutResponseCallbacks begin");
    const Com timeOutCom(1, RESPONSE, TIME_OUT, 1);
    long timeout = (long)millis() - REQUEST_TIMEOUT;
    for (size_t i = 4; i < m_responseCallbackSize; i++){
        if(m_responseCallbackTimes[i] < timeout && m_responseCallbacks[i]){
            if(m_debug)
                m_debug->println("reqeust timed out :(");
            (*m_responseCallbacks[i])(timeOutCom);
            removeResponseCallback(i);
        }
    }
    if(STACK)
        m_debug->println("executeTimeoutResponseCallbacks end");
}

void SerialCom::removeResponseCallback(byte id){
   if(STACK)
         m_debug->println("removeResponseCallback begin");
    m_responseCallbacks[id] = nullptr;
    if(id == m_responseCallbackSize - 1 && m_responseCallbackSize > 0)
        m_responseCallbackSize--;
    if(STACK)
        m_debug->println("removeResponseCallback end");
}

void SerialCom::processRequest(const Com com){
    if(STACK)
        m_debug->println("processRequest begin");
    if(m_debug)
       m_debug->print("received request: ");com.print(m_debug);
    if(m_requestListeners[com.port]){
        char* response = (*m_requestListeners[com.port])(com);
        sendResponse(RESPONSE_PORT, OK, com.requestID, response);
        delete[] response;
    } else{
        if(m_debug)
            m_debug->println("No handler for request on port " + String(com.port) + " found! resuming normal operations");
    }
    if(STACK)
        m_debug->println("processRequest end");
}

void SerialCom::processBroadcast(const Com com){
    if(STACK)
        m_debug->println("processBroadcast begin");
    if(m_broadcastListeners[com.port]){
        (*m_broadcastListeners[com.port])(com);
    } else if(m_debug){
        m_debug->println("No callback found");
    }
    if(STACK)
        m_debug->println("processBroadcast end");
}

void SerialCom::send(byte port, byte type, byte statusCode, byte requestID, const char* msg){
    if(STACK)
        m_debug->println("send begin");
    Com com(port, type, statusCode, requestID, msg);
    char* arr;
    com.getComArr(arr, m_debug);
    if(m_debug)
        m_debug->print("sending: ");com.print(m_debug);
    write(arr);
    delete[] arr;
    if(STACK)
        m_debug->println("send end");
}

void SerialCom::send(const char* com){
    if(STACK)
        m_debug->println("send1 begin");
    write(com);
    if(m_debug)
        m_debug->printf("force sended this massage message: %s\n", com);
    if(STACK)
        m_debug->println("send1 end");
}

void SerialCom::sendResponse(byte port, byte statusCode, byte reqeustID, const char* response){
    if(STACK)
        m_debug->println("sendResponse begin");
    send(port, RESPONSE, statusCode, reqeustID, response);
    if(STACK)
        m_debug->println("sendResponse end");
}

bool SerialCom::isComBufferValid(){
    if(STACK)
        m_debug->println("isComBufferValid begin");
    if(m_comSize > m_startCap + 4){
        if(*(m_comBuffer + m_comSize - 1) != delimiter && m_debug){
            m_debug->println("Com buffer -> End cap invalid");
        }
        if(STACK)
            m_debug->println("isComBufferValid end");
        return *(m_comBuffer + m_comSize - 1) == delimiter;
    }
    if(m_debug)
        m_debug->println("Com buffer invalid");
    if(STACK)
        m_debug->println("isComBufferValid end");
    return false;
}

void SerialCom::sendBroadcast(byte port, const char* msg){
    send(port, BROADCAST, OK, 1, msg);
    delete[] msg;
}

int SerialCom::sendRequest(byte port, const char* msg, responseCallback callback){
    if(STACK)
        m_debug->println("sendRequest begin");
    if(m_debug)
        m_debug->println("starting to send request");
    if(m_responseCallbackSize < 255){
        m_responseCallbacks[m_responseCallbackSize] = callback;
        m_responseCallbackTimes[m_responseCallbackSize] = millis();
        send(port, REQUEST, OK, m_responseCallbackSize, msg);
        m_responseCallbackSize++;
        if(m_debug)
            m_debug->println("request was sended. " + String(m_responseCallbackSize) + " m_responseCallbackSize");
        if(STACK)
            m_debug->println("sendRequest end");
        return m_responseCallbackSize - 1;
    } else if(m_debug){
        m_debug->println("could not send request.. Que is full");
    }
    return 0;
}

bool SerialCom::setRequestListener(byte port, requestListener callback){
    if(STACK)
        m_debug->println("setRequestListener begin");
    if(!isValidPort(port))
        return false;
    if(callback)
        m_requestListeners[port] = callback;
    if(STACK)
        m_debug->println("setRequestListener end");
    return callback;
}

bool SerialCom::setBroadcastListener(byte port, broadcastListener listener){
    if(STACK)
        m_debug->println("setBroadcastListener begin");
    if(!isValidPort(port))
        return false;
    if(listener)
        m_broadcastListeners[port] = listener;
    if(STACK)
        m_debug->println("setBroadcastListener end");
    return listener;
}


bool SerialCom::removeListener(byte type, byte port){
    if(!isValidPort(port))
        return false;
    if(type == REQUEST)
        m_requestListeners[port] = nullptr;
    else if(type == BROADCAST)
        m_broadcastListeners[port] = nullptr;
    else
        return false;
    return true;
}

bool SerialCom::isValidType(byte type){
    if(STACK)
        m_debug->println("isValidType begin");
    if(STACK)
        m_debug->println("isValidType end");
    return type == REQUEST || type == BROADCAST;
}

bool SerialCom::isValidPort(byte port){
    return port > 10;
}

bool SerialCom::available(){
    return Serial.available();
}

void SerialCom::write(const char* input){
    Serial.write(input);
}

char SerialCom::read(){
    return Serial.read();
}