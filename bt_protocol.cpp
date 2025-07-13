#include "bt_protocol.h"
#include <cstring>
#include <stdexcept>

std::string BTProtocol::create_handshake(const std::string& info_hash, const std::string& peer_id) {
    BTHandshake handshake;
    handshake.pstrlen = 19;
    memcpy(handshake.pstr, "BitTorrent protocol", 19);
    memset(handshake.reserved, 0, 8);
    
    if (info_hash.length() >= 20) {
        memcpy(handshake.info_hash, info_hash.c_str(), 20);
    } else {
        memset(handshake.info_hash, 0, 20);
    }
    
    if (peer_id.length() >= 20) {
        memcpy(handshake.peer_id, peer_id.c_str(), 20);
    } else {
        memset(handshake.peer_id, 0, 20);
        memcpy(handshake.peer_id, peer_id.c_str(), peer_id.length());
    }
    
    return std::string(reinterpret_cast<char*>(&handshake), sizeof(BTHandshake));
}

BTMessage BTProtocol::parse_message(const char* data, int length) {
    BTMessage msg;
    if (length < 4) {
        throw std::runtime_error("Message too short");
    }
    
    msg.length = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    
    if (msg.length == 0) {
        msg.id = 255;
        msg.payload = nullptr;
        return msg;
    }
    
    if (length < 5) {
        throw std::runtime_error("Message missing ID");
    }
    
    msg.id = data[4];
    
    if (msg.length > 1) {
        msg.payload = new char[msg.length - 1];
        memcpy(msg.payload, data + 5, msg.length - 1);
    } else {
        msg.payload = nullptr;
    }
    
    return msg;
}

std::string BTProtocol::create_message(unsigned char msg_id, const char* payload, int payload_len) {
    int total_len = 1 + payload_len;
    std::string message(4 + total_len, 0);
    
    message[0] = (total_len >> 24) & 0xFF;
    message[1] = (total_len >> 16) & 0xFF;
    message[2] = (total_len >> 8) & 0xFF;
    message[3] = total_len & 0xFF;
    message[4] = msg_id;
    
    if (payload_len > 0 && payload) {
        memcpy(&message[5], payload, payload_len);
    }
    
    return message;
}

bool BTProtocol::validate_handshake(const char* data, int length) {
    if (length != 68) return false;
    if (data[0] != 19) return false;
    if (memcmp(data + 1, "BitTorrent protocol", 19) != 0) return false;
    return true;
}
