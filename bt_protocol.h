#ifndef BT_PROTOCOL_H
#define BT_PROTOCOL_H

#include <string>

#define BT_MSG_CHOKE 0
#define BT_MSG_UNCHOKE 1
#define BT_MSG_INTERESTED 2
#define BT_MSG_NOT_INTERESTED 3
#define BT_MSG_HAVE 4
#define BT_MSG_BITFIELD 5
#define BT_MSG_REQUEST 6
#define BT_MSG_PIECE 7
#define BT_MSG_CANCEL 8

struct BTHandshake {
    unsigned char pstrlen;
    char pstr[19];
    unsigned char reserved[8];
    unsigned char info_hash[20];
    unsigned char peer_id[20];
};

struct BTMessage {
    unsigned int length;
    unsigned char id;
    char* payload;
};

class BTProtocol {
public:
    static std::string create_handshake(const std::string& info_hash, const std::string& peer_id);
    static BTMessage parse_message(const char* data, int length);
    static std::string create_message(unsigned char msg_id, const char* payload, int payload_len);
    static bool validate_handshake(const char* data, int length);
};

#endif
