#pragma once


class CLogger
{

    static CLogger *instance;

    char pt[128]{};

    CLogger();

public:

    static const char *SERIAL_NUMBER;

    static CLogger *getInstance()
    {
        if (instance == nullptr)
        {
            instance = new CLogger();
        }
        
        return instance;
    }

    

    void log(const char *msg,...);

};