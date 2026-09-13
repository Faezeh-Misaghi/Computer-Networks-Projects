#include <iostream>
#include <fstream>
#include <cstring>
#include <vector> 
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
    if (argc != 4) return 1;

    string ip = argv[1];
    int port = stoi(argv[2]);
    string path = argv[3];

    ifstream inFile(path, ios::binary);
    if (!inFile) return 1;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &receiver_addr.sin_addr);

    vector<Packet> all_packets;
    char buffer[MAX_PAYLOAD_SIZE];
    uint32_t current_seq_num = 0;
    long long total_file_size = 0; 

    while (inFile.read(buffer, MAX_PAYLOAD_SIZE) || inFile.gcount() > 0) {
        size_t bytesRead = inFile.gcount();
        total_file_size += bytesRead; 
        
        Packet pkt;
        memset(&pkt, 0, sizeof(Packet));
        pkt.header.seq_num = current_seq_num;
        pkt.header.length = bytesRead;
        pkt.header.flags = FLAG_DATA;
        memcpy(pkt.payload, buffer, bytesRead);
        pkt.header.checksum = calculate_checksum(pkt);
        
        all_packets.push_back(pkt);
        current_seq_num++;
    }
    inFile.close();

    int total_packets = all_packets.size();
    cout << "Total packets to send: " << total_packets << endl;

    int window_size = 10; 
    
    int base = 0;       
    int next_seq_num = 0;

    int total_packets_sent = 0;
    int total_retransmissions = 0;

    Timer myTimer(1000.0);
    auto start_time = high_resolution_clock::now();

    while (base < total_packets) {
        
        while (next_seq_num < base + window_size && next_seq_num < total_packets) {
            Packet pkt = all_packets[next_seq_num];
            int total_size = sizeof(PacketHeader) + pkt.header.length;
            char network_buffer[total_size];
            serialize(pkt, network_buffer);

            sendto(sock, network_buffer, total_size, 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
            total_packets_sent++;
            
            cout << "Sent packet seq_num: " << next_seq_num << endl;

            if (base == next_seq_num) {
                myTimer.start();
            }
            next_seq_num++;
        }

        char ack_buffer[sizeof(PacketHeader)];
        int bytes_rcvd = recvfrom(sock, ack_buffer, sizeof(ack_buffer), MSG_DONTWAIT, NULL, NULL);

        if (bytes_rcvd > 0) {
            Packet ack_pkt = deserialize(ack_buffer);
            if ((ack_pkt.header.flags & FLAG_ACK) && calculate_checksum(ack_pkt) == ack_pkt.header.checksum) {
                
                uint32_t ack_num = ack_pkt.header.ack_num;
                cout << "Received ACK for expected seq_num: " << ack_num << endl;
                if (ack_num > base) {
                    base = ack_num; 
                    if (base == next_seq_num) {
                        myTimer.stop();
                    } else {
                        myTimer.start();
                    }
                }
            }
        } 
        else {
            if (myTimer.is_timeout()) {
                cout << "!!! Timeout occurred for base " << base << " !!!" << endl;
                myTimer.start();
                next_seq_num = base;
                total_retransmissions++; 
            }
            usleep(1000); 
        }
    }

    auto end_time = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end_time - start_time);
    close(sock);

    cout << "\nTransfer Complete" << endl;
    cout << "Total file size: " << total_file_size << " bytes" << endl;
    cout << "Total time: " << time_span.count() << " seconds" << endl;
    cout << "Total packets sent: " << total_packets_sent << endl;
    cout << "Total timeout events: " << total_retransmissions << endl;
    double throughput_kbps = (total_file_size / 1024.0) / time_span.count();
    cout << "Throughput: " << throughput_kbps << " KB/s" << endl;

    return 0;
}