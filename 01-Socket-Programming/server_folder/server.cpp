#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h> 
#include <netinet/in.h>  
#include <unistd.h> 
#include <vector>    
#include <algorithm>
#include <dirent.h>
#include <fstream>

using namespace std;

void handle_command(int c_socket, string command, vector<int>& online_clients, fd_set& master_set) {

    string cmd;
    string msg;
    size_t space_pos = command.find(' ');
    if(space_pos != string::npos){
        cmd = command.substr(0, space_pos);
        msg = command.substr(space_pos + 1);
    } else {
        cmd = command;
    }

    if(cmd == "MSG"){
        if(msg.empty()) {
            string err = "Please write a message!\n";
            send(c_socket, err.c_str(), err.length(), 0);
            return;
        }
        string broadcast_msg = "Message from Client " + to_string(c_socket) + " : " + msg + "\n";
        for(size_t i = 0; i < online_clients.size(); i++) {
            int target = online_clients[i];
            if(target != c_socket) {
                send(target, broadcast_msg.c_str(), broadcast_msg.length(), 0);
            }
        }
        return ;
    }

    if(cmd == "PM"){
        if(msg.empty()) {
            string err = "Please specify target and message!\n";
            send(c_socket, err.c_str(), err.length(), 0);
            return;
        }
        size_t pm_space_pos = msg.find(' ');
        if(pm_space_pos != string::npos) {

            string target_id = msg.substr(0, pm_space_pos);
            string private_msg = msg.substr(pm_space_pos + 1);

            int target_socket = stoi(target_id);
            bool is_online = false;
            for (size_t i = 0; i < online_clients.size(); i++) {
                if(online_clients[i] == target_socket) {
                    is_online = true;
                    break;
                }
            }
            if(is_online) {
                string formatted_msg = "Private message from Client " + to_string(c_socket) + " : " + private_msg + "\n";
                send(target_socket, formatted_msg.c_str(), formatted_msg.length(), 0);
            } else {
                string err = "User offline.\n";
                send(c_socket, err.c_str(), err.length(), 0);
            }
        } else {
            string err = "Message is missing!\n";
            send(c_socket, err.c_str(), err.length(), 0);
        }
        return;
    }
    if(cmd == "USERS"){

        string users_list = "\n --- Online Users ---\n";
        for (size_t i = 0; i < online_clients.size(); i++) {
            users_list += "- Client " + to_string(online_clients[i]);
            if (online_clients[i] == c_socket) {
                users_list += " (You)";
            }
            users_list += "\n";
        }
        send(c_socket, users_list.c_str(), users_list.length(), 0);

        return;
    }
    if(cmd == "LIST"){
        string file_list = "--- Available Files ---\n";
        DIR *dir;
        struct dirent *ent;
        
        if ((dir = opendir("./")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                string file_name = ent->d_name;
                
                if (file_name != "." && file_name != "..") {
                    file_list += "- " + file_name + "\n";
                }
            }
            closedir(dir);
        } else {
            file_list = "Could not open server directory.\n";
        }
        
        file_list += "-----------------------\n";

        send(c_socket, file_list.c_str(), file_list.length(), 0);
        return;
    }
    if(cmd == "GET"){
        string filename = msg; 

        if (filename.empty()) {
            string err = "Please specify a filename (e.g., GET mytext.txt)\n";
            send(c_socket, err.c_str(), err.length(), 0);
        } else {
            ifstream file(filename, ios::binary | ios::ate);
            if (!file.is_open()) {
                string err = "File not found on server.\n";
                send(c_socket, err.c_str(), err.length(), 0);
            } else {
                streamsize file_size = file.tellg();
                file.seekg(0, ios::beg);
                string file_info_msg = "FILE_INFO " + to_string(file_size) + " " + filename + "\n";
                send(c_socket, file_info_msg.c_str(), file_info_msg.length(), 0);
                
                usleep(100000); // 100 millisecond

                char buffer[4096];
                while (file.read(buffer, sizeof(buffer))) {
                    send(c_socket, buffer, file.gcount(), 0);
                }
                if (file.gcount() > 0) {
                    send(c_socket, buffer, file.gcount(), 0);
                }
                file.close();
                cout << "File '" << filename << "' sent to client " << c_socket << endl;
            }
        }
        return;
    }
    if(cmd == "PUT"){
        if(msg.empty()) {
            string err = "Invalid PUT command format.\n";
            send(c_socket, err.c_str(), err.length(), 0);
            return;
        }

        size_t space_pos = msg.find(' ');
        if(space_pos == string::npos) {
            string err = "Missing file size or filename.\n";
            send(c_socket, err.c_str(), err.length(), 0);
            return;
        }

        long long file_size = stoll(msg.substr(0, space_pos));
        string filename = msg.substr(space_pos + 1);
        string save_filename = "up_" + to_string(c_socket) + "_" + filename;
        cout << "Receiving file '" << filename << "' (" << file_size << " bytes) from Client " << c_socket << "..." << endl;
        ofstream outfile(save_filename, ios::binary);
        if(!outfile.is_open()) {
            string err = "Server could not create file.\n";
            send(c_socket, err.c_str(), err.length(), 0);
            return;
        }

        long long total_received = 0;
        char file_buf[4096];
        while (total_received < file_size) {
            int bytes_to_recv = sizeof(file_buf);
            if (file_size - total_received < (long long)sizeof(file_buf)) {
                bytes_to_recv = file_size - total_received;
            }
            int r = recv(c_socket, file_buf, bytes_to_recv, 0);
            if (r <= 0) {
                cout << "Client disconnected during upload!" << endl;
                break;
            }
            outfile.write(file_buf, r);
            total_received += r;
        }

        outfile.close();
        cout << "Upload complete: '" << save_filename << "'" << endl;
        string success_msg = "File '" + filename + "' uploaded successfully as '" + save_filename + "'.\n";
        send(c_socket, success_msg.c_str(), success_msg.length(), 0);
        
        return;
    }
    if(cmd == "QUIT"){
        string quit_msg = "Goodbye!\n";
        send(c_socket, quit_msg.c_str(), quit_msg.length(), 0);
        string left_msg = "[System]: Client " + to_string(c_socket) + " has left the chat.\n";
        for (size_t i = 0; i < online_clients.size(); i++) {
            if (online_clients[i] != c_socket) {
                send(online_clients[i], left_msg.c_str(), left_msg.length(), 0);
            }
        }

        close(c_socket);
        FD_CLR(c_socket, &master_set);
        for (auto it = online_clients.begin(); it != online_clients.end(); ++it) {
            if (*it == c_socket) {
                online_clients.erase(it);
                break;
            }
        }
        
        cout << "Client " << c_socket << " disconnected" << endl;
        return;
    }

    string invalid_string = "Invalid Command!";
    send(c_socket, invalid_string.c_str(), invalid_string.length(), 0);

}

int main(){

    int server_s = socket(AF_INET, SOCK_STREAM, 0);
    if(server_s < 0) {return 1;}

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    int bind_check = bind(server_s, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(bind_check < 0){
        cerr << "Bind faild!" << endl;
        return 1;
    }

    int listen_check = listen(server_s, 5);
    if(listen_check < 0){
        cerr << "Listen failed!" << endl;
        return 1;
    }

    fd_set rfds; 
    int max_socket = server_s;
    FD_ZERO(&rfds);
    FD_SET(server_s, &rfds); 
    
    vector<int> online_clients;

    while(true){

        fd_set read_fds = rfds; 
        int active = select(max_socket + 1, &read_fds, NULL, NULL, NULL);
        if(active < 0){ break;}

        int new_client = FD_ISSET(server_s, &read_fds);
        if(new_client){
            int client = accept(server_s, NULL, NULL);
            if(client >= 0){
                FD_SET(client, &rfds);
                online_clients.push_back(client);
                if(client > max_socket){
                    max_socket = client;
                }
                string welcome_msg = "Connected to server successfully. Your ID is: " + to_string(client) + "\n";
                send(client, welcome_msg.c_str(), welcome_msg.length(), 0);
            }
        }
        for(size_t i = 0; i < online_clients.size(); i++){
            int current_client = online_clients[i];
            int send_msg = FD_ISSET(current_client, &read_fds);
            if(send_msg){
                char buffer[1024] = {0};
                int received = recv(current_client, buffer, 1024, 0);
                if(received <= 0){
                    close(current_client);
                    FD_CLR(current_client, &rfds); 
                    online_clients.erase(online_clients.begin() + i); 
                    i--; 
                } else {
                    string client_message(buffer);
                    client_message.erase(client_message.find_last_not_of(" \n\r\t") + 1); 
                    handle_command(current_client, client_message, online_clients, rfds);
                }
            }
        }
    }

    close(server_s);
    return 0;
}
