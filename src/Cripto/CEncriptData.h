#pragma once

#include <cstdint>

char *CEncriptData::key = "secret";

class CEncriptData
{
    static char* key;

public:

   static void encriptData(char *data, int len, char *key, int keyLen)
   {

    for (size_t i = 0; i < len; ++i) {

        data[i] ^= key[i % keyLen];
    }

   }

   static void decriptionData(char *data, int len, char *key, int keyLen)
   {

    for (size_t i = 0; i < len; ++i) {

        data[i] ^= key[i % keyLen];
    }

   }

}