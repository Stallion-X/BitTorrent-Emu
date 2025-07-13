#include "bencode.h"
#include <sstream>
#include <stdexcept>

std::shared_ptr<BencodeValue> BencodeValue::create_string(const std::string& str) {
    auto value = std::make_shared<BencodeValue>(BencodeType::STRING);
    value->string_value = str;
    return value;
}

std::shared_ptr<BencodeValue> BencodeValue::create_integer(long long val) {
    auto value = std::make_shared<BencodeValue>(BencodeType::INTEGER);
    value->int_value = val;
    return value;
}

std::shared_ptr<BencodeValue> BencodeValue::create_list() {
    return std::make_shared<BencodeValue>(BencodeType::LIST);
}

std::shared_ptr<BencodeValue> BencodeValue::create_dictionary() {
    return std::make_shared<BencodeValue>(BencodeType::DICTIONARY);
}

std::shared_ptr<BencodeValue> BencodeParser::parse(const std::string& data) {
    size_t pos = 0;
    return parse_value(data, pos);
}

std::shared_ptr<BencodeValue> BencodeParser::parse_value(const std::string& data, size_t& pos) {
    if (pos >= data.length()) {
        throw std::runtime_error("Unexpected end of data");
    }
    
    char c = data[pos];
    if (c >= '0' && c <= '9') {
        return parse_string(data, pos);
    } else if (c == 'i') {
        return parse_integer(data, pos);
    } else if (c == 'l') {
        return parse_list(data, pos);
    } else if (c == 'd') {
        return parse_dictionary(data, pos);
    } else {
        throw std::runtime_error("Invalid bencode data");
    }
}

std::shared_ptr<BencodeValue> BencodeParser::parse_string(const std::string& data, size_t& pos) {
    size_t colon_pos = data.find(':', pos);
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Invalid string format");
    }
    
    std::string length_str = data.substr(pos, colon_pos - pos);
    int length = std::stoi(length_str);
    pos = colon_pos + 1;
    
    if (pos + length > data.length()) {
        throw std::runtime_error("String length exceeds data");
    }
    
    std::string str = data.substr(pos, length);
    pos += length;
    
    return BencodeValue::create_string(str);
}

std::shared_ptr<BencodeValue> BencodeParser::parse_integer(const std::string& data, size_t& pos) {
    pos++;
    size_t end_pos = data.find('e', pos);
    if (end_pos == std::string::npos) {
        throw std::runtime_error("Invalid integer format");
    }
    
    std::string int_str = data.substr(pos, end_pos - pos);
    long long val = std::stoll(int_str);
    pos = end_pos + 1;
    
    return BencodeValue::create_integer(val);
}

std::shared_ptr<BencodeValue> BencodeParser::parse_list(const std::string& data, size_t& pos) {
    pos++;
    auto list = BencodeValue::create_list();
    
    while (pos < data.length() && data[pos] != 'e') {
        auto value = parse_value(data, pos);
        list->list_value.push_back(value);
    }
    
    if (pos >= data.length() || data[pos] != 'e') {
        throw std::runtime_error("Invalid list format");
    }
    pos++;
    
    return list;
}

std::shared_ptr<BencodeValue> BencodeParser::parse_dictionary(const std::string& data, size_t& pos) {
    pos++;
    auto dict = BencodeValue::create_dictionary();
    
    while (pos < data.length() && data[pos] != 'e') {
        auto key = parse_string(data, pos);
        auto value = parse_value(data, pos);
        dict->dict_value[key->string_value] = value;
    }
    
    if (pos >= data.length() || data[pos] != 'e') {
        throw std::runtime_error("Invalid dictionary format");
    }
    pos++;
    
    return dict;
}

std::string BencodeParser::encode(const std::shared_ptr<BencodeValue>& value) {
    switch (value->type) {
        case BencodeType::STRING:
            return encode_string(value->string_value);
        case BencodeType::INTEGER:
            return encode_integer(value->int_value);
        case BencodeType::LIST:
            return encode_list(value->list_value);
        case BencodeType::DICTIONARY:
            return encode_dictionary(value->dict_value);
        default:
            throw std::runtime_error("Unknown bencode type");
    }
}

std::string BencodeParser::encode_string(const std::string& str) {
    return std::to_string(str.length()) + ":" + str;
}

std::string BencodeParser::encode_integer(long long val) {
    return "i" + std::to_string(val) + "e";
}

std::string BencodeParser::encode_list(const std::vector<std::shared_ptr<BencodeValue>>& list) {
    std::string result = "l";
    for (const auto& item : list) {
        result += encode(item);
    }
    result += "e";
    return result;
}

std::string BencodeParser::encode_dictionary(const std::map<std::string, std::shared_ptr<BencodeValue>>& dict) {
    std::string result = "d";
    for (const auto& pair : dict) {
        result += encode_string(pair.first);
        result += encode(pair.second);
    }
    result += "e";
    return result;
}
