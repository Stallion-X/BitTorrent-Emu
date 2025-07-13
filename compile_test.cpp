#include "bencode.h"
#include <iostream>

int main() {
    try {
        auto dict = BencodeValue::create_dictionary();
        dict->dict_value["test"] = BencodeValue::create_string("hello");
        std::string encoded = BencodeParser::encode(dict);
        std::cout << "Encoded: " << encoded << std::endl;
        
        auto parsed = BencodeParser::parse(encoded);
        std::cout << "Parsed back: " << parsed->dict_value["test"]->string_value << std::endl;
        
        std::cout << "Bencode library works!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
