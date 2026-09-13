#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>  
#include <arpa/inet.h>   
#include <unistd.h> 
#include <sys/select.h>
#include <sstream>
#include <fstream>    

using namespace std;

int main(){

    int client_s = socket(AF_INET, SOCK_STREAM, 0);
    if(client_s < 0) {return 1;}

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    int connection = connect(client_s, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(connection < 0){
        cerr << "Connection failed!" << endl;
        close(client_s);
        return 1;
    }

    cout << "> " << flush;

    fd_set read_fds;
    char buffer[1024];

    while (true) {
        FD_ZERO(&read_fds);              
        FD_SET(STDIN_FILENO, &read_fds); 
        FD_SET(client_s, &read_fds);    

        int active = select(client_s + 1, &read_fds, nullptr, nullptr, nullptr);
        if (active < 0) { break;}

        int server = FD_ISSET(client_s, &read_fds);
        int keyboard = FD_ISSET(STDIN_FILENO, &read_fds);

        if (server) {
            memset(buffer, 0, sizeof(buffer));
            int received = recv(client_s, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) { break;}
            string server_msg(buffer);

            if (server_msg.substr(0, 10) == "FILE_INFO ") {
                stringstream ss(server_msg);
                string dummy, filename;
                long long file_size;
                ss >> dummy >> file_size >> filename;
                cout << "\n[System]: Starting download: " << filename << " (" << file_size << " bytes)..." << endl;

                ofstream outfile("dl_" + filename, ios::binary);                
                long long total_received = 0;
                char file_buf[4096];

                while (total_received < file_size) {
                    int bytes_to_recv = sizeof(file_buf);
                    if (file_size - total_received < sizeof(file_buf)) {
                        bytes_to_recv = file_size - total_received;
                    }
                    int r = recv(client_s, file_buf, bytes_to_recv, 0);
                    if (r <= 0) {
                        cout << "Connection lost during download." << endl;
                        break;
                    }
                    outfile.write(file_buf, r);
                    total_received += r;
                }

                outfile.close();
                cout << "[System]: Download complete! Saved as 'dl_" << filename << "'\n> " << flush;
            } 
            else {
                cout << "[Server]: " << server_msg << "\n> " << flush;
            }
        }

        if (keyboard) {
            string input;
            getline(cin, input);

            if (input.substr(0, 4) == "PUT ") {
                string filename = input.substr(4);
                ifstream infile(filename, ios::binary | ios::ate);
                
                if (!infile.is_open()) {
                    cout << "File not found on your system!" << endl;
                    cout << "> " << flush;
                } else {
                    long long file_size = infile.tellg();
                    infile.seekg(0); 
                    cout << "[System]: Starting upload: " << filename << " (" << file_size << " bytes)..." << endl;
                    string header = "PUT " + to_string(file_size) + " " + filename;
                    send(client_s, header.c_str(), header.length(), 0);
                    usleep(50000); 

                    char file_buf[4096];
                    long long total_sent = 0;
                    while (total_sent < file_size) {
                        infile.read(file_buf, sizeof(file_buf));
                        int bytes_read = infile.gcount();
                        int s = send(client_s, file_buf, bytes_read, 0);
                        if (s <= 0) {
                            cout << "Connection lost during upload." << endl;
                            break;
                        }
                        total_sent += s;
                    }
                    infile.close();
                    cout << "[System]: Upload process finished!\n> " << flush;
                }
            } 
            else {
                send(client_s, input.c_str(), input.length(), 0);
                cout << "> " << flush;
            }
        }
    }
    close(client_s);
    return 0;
}