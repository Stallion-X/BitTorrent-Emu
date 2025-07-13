#include "bencode.h"
#include <iostream>
#include <cassert>

using namespace std;

void test_string_encoding() {
    auto str_val = BencodeValue::create_string("hello");
    string encoded = BencodeParser::encode(str_val);
    assert(encoded == "5:hello");
    cout << "String encoding test passed" << endl;
}

void test_integer_encoding() {
    auto int_val = BencodeValue::create_integer(42);
    string encoded = BencodeParser::encode(int_val);
    assert(encoded == "i42e");
    cout << "Integer encoding test passed" << endl;
}

void test_list_encoding() {
    auto list_val = BencodeValue::create_list();
    list_val->list_value.push_back(BencodeValue::create_string("spam"));
    list_val->list_value.push_back(BencodeValue::create_string("eggs"));
    string encoded = BencodeParser::encode(list_val);
    assert(encoded == "l4:spam4:eggse");
    cout << "List encoding test passed" << endl;
}

void test_dictionary_encoding() {
    auto dict_val = BencodeValue::create_dictionary();
    dict_val->dict_value["cow"] = BencodeValue::create_string("moo");
    dict_val->dict_value["spam"] = BencodeValue::create_string("eggs");
    string encoded = BencodeParser::encode(dict_val);
    assert(encoded == "d3:cow3:moo4:spam4:eggse");
    cout << "Dictionary encoding test passed" << endl;
}

void test_parsing() {
    string data = "d3:cow3:moo4:spam4:eggse";
    auto parsed = BencodeParser::parse(data);
    assert(parsed->type == BencodeType::DICTIONARY);
    assert(parsed->dict_value["cow"]->string_value == "moo");
    assert(parsed->dict_value["spam"]->string_value == "eggs");
    cout << "Parsing test passed" << endl;
}

int main() {
    try {
        test_string_encoding();
        test_integer_encoding();
        test_list_encoding();
        test_dictionary_encoding();
        test_parsing();
        cout << "All bencode tests passed!" << endl;
    } catch (const exception& e) {
        cout << "Test failed: " << e.what() << endl;
        return 1;
    }
    return 0;
}
