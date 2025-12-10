#pragma once

#include "cryptoPP/cryptopp880/cryptlib.h"
#include "cryptoPP/cryptopp880/rijndael.h"
#include "cryptoPP/cryptopp880/modes.h"
#include "cryptoPP/cryptopp880/files.h"
#include "cryptoPP/cryptopp880/osrng.h"
#include "cryptoPP/cryptopp880/hex.h"
#include <string>
#include <iostream>
#include <iomanip>

using namespace CryptoPP;

const byte KEY[AES::DEFAULT_KEYLENGTH] = {
    0x9c, 0x17, 0xd1, 0xaf, 0xa4, 0xe2, 0xfd, 0x60,
    0xf0, 0x3a, 0x9a, 0x1b, 0x12, 0x01, 0x83, 0xa1};

const byte IV[AES::BLOCKSIZE] = {
    0x91, 0x51, 0x57, 0x69, 0x0c, 0xbe, 0x74, 0x6b,
    0xa8, 0x36, 0xe5, 0xa3, 0x36, 0xcf, 0xd1, 0x3a};

std::string encrypt(std::string plainText)
{
    std::string cipher;

    CBC_Mode<AES>::Encryption e;
    e.SetKeyWithIV(KEY, AES::DEFAULT_KEYLENGTH, IV);

    StringSource s(plainText, true,
                   new StreamTransformationFilter(e, new StringSink(cipher)));

    return cipher;
}

std::string decrypt(std::string cipherText)
{
    std::string decrypted;

    CBC_Mode<AES>::Decryption d;
    d.SetKeyWithIV(KEY, AES::DEFAULT_KEYLENGTH, IV);

    StringSource s(cipherText, true,
                   new StreamTransformationFilter(d, new StringSink(decrypted)));

    return decrypted;
}

/*
i used this function once to generate const KEY and IV;
inline void generateKeyAndIV()
{
    AutoSeededRandomPool prng;

    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(AES::BLOCKSIZE);

    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());

    std::cout << "// Generated Key:" << std::endl;
    std::cout << "const byte KEY[AES::DEFAULT_KEYLENGTH] = {" << std::endl
              << "    ";
    for (size_t i = 0; i < key.size(); i++)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << (int)key[i];
        if (i < key.size() - 1)
            std::cout << ", ";
        if ((i + 1) % 8 == 0 && i < key.size() - 1)
            std::cout << std::endl
                      << "    ";
    }
    std::cout << std::endl
              << "};" << std::endl
              << std::endl;

    std::cout << "// Generated IV:" << std::endl;
    std::cout << "const byte FIXED_IV[AES::BLOCKSIZE] = {" << std::endl
              << "    ";
    for (size_t i = 0; i < iv.size(); i++)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << (int)iv[i];
        if (i < iv.size() - 1)
            std::cout << ", ";
        if ((i + 1) % 8 == 0 && i < iv.size() - 1)
            std::cout << std::endl
                      << "    ";
    }
    std::cout << std::endl
              << "};" << std::endl;
}
 */