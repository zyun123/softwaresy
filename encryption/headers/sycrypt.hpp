#ifndef SYCRYPT_HPP
#define SYCRYPT_HPP

#include <string>
#include <vector>

class SYKey;

class SYEncryptor {
    public:
        SYEncryptor();
        ~SYEncryptor();
        int setPubKey(std::string pubkeyfile);
        int encrypt(std::vector<unsigned char> buffer_in, 
                    std::vector<unsigned char>& buffer_out);
    private:
        SYKey *sy_key;
};

class SYDecryptor {
    public:
        SYDecryptor();
        ~SYDecryptor();
        int setKey(std::string keyfile, std::string pass);
        int decrypt(std::vector<unsigned char> buffer_in, 
                    std::vector<unsigned char>& buffer_out);
    private:
        SYKey *sy_key;
};

#endif //SYCRYPT_HPP