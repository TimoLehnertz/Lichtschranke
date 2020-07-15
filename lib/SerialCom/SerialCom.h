#ifndef SerialCom_h
#define SerialCom_h

#define SoftwareSerialDebug 0

#include "SerialInterface.h"
#if SoftwareSerialDebug == 1
#include <SoftwareSerial.h>
#endif

#define DELIMITER '\n'

//Structure of com: 1 byte port, 1 byte type, 1 byte statusCode, 1 byte requestID, 1 - 249 bytes message(must be null terminated), 1 byte delimiter

struct Com{
    Com(char com[], byte start, byte delimiter){
        message[254] = 0; //Safty null termination
        port = com[start];
        type = com[start + 1];
        statusCode = com[start + 2];
        requestID = com[start + 3];
        com[delimiter] = '\0';
        if(delimiter - start > 4){
            strcpy(message, (com + start + 4));
        }
    }

    Com() {}

    Com(byte port, byte type, byte statusCode, byte requestID, const char* msg = "Timeout") : port(port), type(type), statusCode(statusCode), requestID(requestID){
        strcpy(message, msg);
    }

    byte port;
    byte type;
    byte statusCode;
    byte requestID;
    char message[255];

    char* getComArr(char* comArr, SoftwareSerial* debug) {
        //char* comArr = new char[255]; old
        //comArr[0] = start_CAP; old
        comArr[0] = port;
        comArr[1] = type;
        comArr[2] = statusCode;
        comArr[3] = requestID;
        strcpy(&comArr[4], message);
        byte length = strlen(comArr);
        comArr[length] = DELIMITER;
        comArr[length + 1] = 0;
        return comArr;
    }
        #if SoftwareSerialDebug == 1
    void print(SoftwareSerial* debug) const{
        if(debug)
            debug->printf("Debug Com: port: %i, type: %i, statusCode: %i, requestID: %i, message: %s\n", port, type, statusCode, requestID, message);
    }
    #endif
};

typedef void(*broadcastListener)(const Com&);
typedef char*(*requestListener)(const Com&);
typedef void(*responseCallback)(const Com&);

class SerialCom{
    public:

        SerialCom(SerialInterface* comInterface);

        void begin();//-------------------------------------------------------------aestetic
        void handleComunication();//                                                to put in loop
        void sendBroadcast(byte port, const char* msg);//---------------------------send broadcast to all listening members on this port
        int sendRequest(byte port, const char* msg, responseCallback callback);//   send an asyncron request to this port and call callback when request is answered or timed out returns the requests id or 0 if failed
        bool setRequestListener(byte port, requestListener);//----------------------set the callback for incoming requests on this port Callback signature: char*(*)(const Com&)
        bool setBroadcastListener(byte port, broadcastListener);//                  set the callback for incoming broadcasts on this port Callback signature: void(*)(const Com&)
        bool removeListener(byte type, byte port);//--------------------------------remove the callback from this port and this type(note a port can have seperate callbacks on one port for requests and broadcasts)
        void setDebug(SoftwareSerial&);//                                           parse this an instance of softwareserial for debug logging. To disable after activating, pass a nullptr
        void setDebug(void*);

        int REQUEST_TIMEOUT = 1000;

        static const byte REQUEST = 4;
        static const byte RESPONSE = 5;
        static const byte BROADCAST = 6;

        static const byte ERROR = 4;
        static const byte OK = 5;
        static const byte NOT_FOUND = 6;
        static const byte TIME_OUT = 7;
        

    private:
        SerialInterface* m_ComInterface;
        void send(byte port, byte type, byte statusCode, byte requestinfo, const char* msg);
        void send(const char*);
        void sendQue();
        char m_comBuffer[255];
        int m_comSize;
        broadcastListener m_broadcastListeners[255];
        requestListener m_requestListeners[255];
        int m_listenerSize;
        bool isValidPort(byte);
        char m_buffer;
        int m_startCap = 0;
        void processCombuffer();
        #if SoftwareSerialDebug == 1
        SoftwareSerial* m_debug;
        #endif
        bool isComBufferValid();
        void processRequest(const Com);
        void processBroadcast(const Com);
        bool isValidType(byte);
        void sendResponse(byte, byte, byte, const char* = "");
        char* m_sendQue[50];
        byte m_sendQueSize = 0;
        responseCallback m_responseCallbacks[255];
        byte* m_responseCallbackPorts[255];
        long m_responseCallbackTimes[255];
        byte m_responseCallbackSize = 4;
        void removeResponseCallback(byte);
        void executeTimeoutResponseCallbacks();

        static const byte RESPONSE_PORT = 10;
        static const char delimiter = DELIMITER;
        //static const char END_CAP = end_CAP; old
};

#endif
