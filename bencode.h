#ifndef BENCODE_H
#define BENCODE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

enum class BencodeType {
    STRING,
    INTEGER,
    LIST,
    DICTIONARY
};

class BencodeValue {
public:
    BencodeType type;
    std::string string_value;
    long long int_value;
    std::vector<std::shared_ptr<BencodeValue>> list_value;
    std::map<std::string, std::shared_ptr<BencodeValue>> dict_value;

    BencodeValue(BencodeType t) : type(t), int_value(0) {}
    
    static std::shared_ptr<BencodeValue> create_string(const std::string& str);
    static std::shared_ptr<BencodeValue> create_integer(long long val);
    static std::shared_ptr<BencodeValue> create_list();
    static std::shared_ptr<BencodeValue> create_dictionary();
};

class BencodeParser {
public:
    static std::shared_ptr<BencodeValue> parse(const std::string& data);
    static std::string encode(const std::shared_ptr<BencodeValue>& value);

private:
    static std::shared_ptr<BencodeValue> parse_value(const std::string& data, size_t& pos);
    static std::shared_ptr<BencodeValue> parse_string(const std::string& data, size_t& pos);
    static std::shared_ptr<BencodeValue> parse_integer(const std::string& data, size_t& pos);
    static std::shared_ptr<BencodeValue> parse_list(const std::string& data, size_t& pos);
    static std::shared_ptr<BencodeValue> parse_dictionary(const std::string& data, size_t& pos);
    
    static std::string encode_string(const std::string& str);
    static std::string encode_integer(long long val);
    static std::string encode_list(const std::vector<std::shared_ptr<BencodeValue>>& list);
    static std::string encode_dictionary(const std::map<std::string, std::shared_ptr<BencodeValue>>& dict);
};

#endif
