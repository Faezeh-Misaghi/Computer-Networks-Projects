#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <cstring>
#include <vector>

const uint8_t FLAG_DATA = 0x01;
const uint8_t FLAG_ACK  = 0x02;
const uint8_t FLAG_FIN  = 0x04;

using namespace std;

struct PacketHeader {
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t length;   
    uint16_t checksum;
    uint8_t flags;
};

const int MAX_PAYLOAD_SIZE = 1024;

struct Packet {
    PacketHeader header;
    char payload[MAX_PAYLOAD_SIZE];
};

uint16_t calculate_checksum(Packet pkt) {
    long sum = 0; 

    sum += pkt.header.seq_num;
    sum += pkt.header.ack_num;
    sum += pkt.header.length;
    sum += pkt.header.flags;

    for (int i = 0; i < pkt.header.length; i++) {
        sum += pkt.payload[i];
    }
    return sum % 65536;
}

void serialize(Packet pkt, char* buffer) {
    memcpy(buffer, &pkt.header, sizeof(PacketHeader));
    memcpy(buffer + sizeof(PacketHeader), pkt.payload, pkt.header.length);
}

Packet deserialize(char* buffer) {
    Packet pkt;
    memcpy(&pkt.header, buffer, sizeof(PacketHeader));
    memcpy(pkt.payload, buffer + sizeof(PacketHeader), pkt.header.length);
    
    return pkt;
}

#endif