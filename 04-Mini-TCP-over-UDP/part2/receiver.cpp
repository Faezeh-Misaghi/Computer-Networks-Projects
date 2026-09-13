#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "packet.hpp"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        return 1;
    }

    int port = atoi(argv[1]);
    string path = argv[2];

    ofstream outFile(path, ios::binary);
    if (!outFile) {
        cerr << "Error creating output file!" << endl;
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in receiver_addr, sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
    cout << "Receiver is listening on port " << port << "..." << endl;

    char buffer[sizeof(PacketHeader) + MAX_PAYLOAD_SIZE];
    uint32_t expected_seq_num = 0;

    while (true) {
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_addr, &sender_len);
        if (bytes_received <= 0) continue;

        Packet received_pkt = deserialize(buffer);
        uint16_t calculated_checksum = calculate_checksum(received_pkt);
        
        if (calculated_checksum == received_pkt.header.checksum) {

            if (received_pkt.header.seq_num == expected_seq_num) {
                outFile.write(received_pkt.payload, received_pkt.header.length);
                outFile.flush(); 
                cout << "Received in-order seq_num: " << received_pkt.header.seq_num << endl;
                expected_seq_num++; 
            } else {
                cout << "Out-of-order " << expected_seq_num << endl;
            }
            Packet ack_pkt;
            memset(&ack_pkt, 0, sizeof(Packet));
            ack_pkt.header.seq_num = 0;
            ack_pkt.header.ack_num = expected_seq_num;
            ack_pkt.header.length = 0; 
            ack_pkt.header.flags = FLAG_ACK;
            ack_pkt.header.checksum = calculate_checksum(ack_pkt);

            char ack_buffer[sizeof(PacketHeader)];
            serialize(ack_pkt, ack_buffer);

            sendto(sock, ack_buffer, sizeof(ack_buffer), 0, (struct sockaddr*)&sender_addr, sender_len);
            cout << "Sent Cumulative ACK for expected seq_num: " << expected_seq_num << endl;
            
        } else {
            cout << "Checksum mismatch" << endl;
        }
    }

    outFile.close();
    close(sock);
    return 0;
}