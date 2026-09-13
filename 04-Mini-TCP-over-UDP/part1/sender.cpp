#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono> 
#include "packet.hpp"
#include "timer.hpp" 

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) {

    if (argc != 4) {
        return 1;
    }

    string ip = argv[1];
    int port = stoi(argv[2]);
    string path = argv[3];

    ifstream inFile(path, ios::binary);
    if (!inFile) {
        cerr << "Error opening input file!" << endl;
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &receiver_addr.sin_addr);

    char buffer[MAX_PAYLOAD_SIZE];
    uint32_t current_seq_num = 0;

    int total_packets_sent = 0;
    int total_retransmissions = 0;

    auto start_time = high_resolution_clock::now();

    Timer myTimer(1000.0); 

    while (inFile.read(buffer, MAX_PAYLOAD_SIZE) || inFile.gcount() > 0) {
        size_t bytesRead = inFile.gcount();
        
        Packet pkt;
        memset(&pkt, 0, sizeof(Packet));
        pkt.header.seq_num = current_seq_num; 
        pkt.header.length = bytesRead;
        pkt.header.flags = FLAG_DATA;
        memcpy(pkt.payload, buffer, bytesRead);
        pkt.header.checksum = calculate_checksum(pkt);

        int total_size = sizeof(PacketHeader) + bytesRead;
        char network_buffer[total_size];

        serialize(pkt, network_buffer);
        bool ack_received = false;

        sendto(sock, network_buffer, total_size, 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
        total_packets_sent++;
        myTimer.start(); 

        cout << "Send packet seq_num: " << current_seq_num << endl;

        while (!ack_received) {
            char ack_buffer[sizeof(PacketHeader)];
            int bytes_rcvd = recvfrom(sock, ack_buffer, sizeof(ack_buffer), MSG_DONTWAIT, NULL, NULL);

            if (bytes_rcvd > 0) {
                Packet ack_pkt = deserialize(ack_buffer);
                if ((ack_pkt.header.flags & FLAG_ACK) && 
                    ack_pkt.header.ack_num == current_seq_num &&
                    calculate_checksum(ack_pkt) == ack_pkt.header.checksum) {
                    
                    cout << "ACK received for seq_num: " << current_seq_num << endl;
                    ack_received = true; 
                    myTimer.stop(); 
                }
            } else {

                if (myTimer.is_timeout()) {
                    cout << "Timeout. Resending packet seq_num: " << current_seq_num << endl;
                    sendto(sock, network_buffer, total_size, 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
                    total_retransmissions++;
                    total_packets_sent++;
                    myTimer.start(); 
                }
                usleep(1000); 
            }
        }
        current_seq_num++;
    }

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end_time - start_time);

    inFile.close();
    close(sock);

    cout << "\nTransfer Complete" << endl;
    cout << "Total time: " << time_span.count() << " seconds" << endl;
    cout << "Total packets sent: " << total_packets_sent << endl;
    cout << "Total retransmissions: " << total_retransmissions << endl;

    return 0;
}