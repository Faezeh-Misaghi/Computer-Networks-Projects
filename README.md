
Welcome to the repository for my Computer Networks course projects. This collection demonstrates hands-on experience with network programming, simulation, and routing protocols. The projects range from low-level socket programming in C++ to advanced network performance evaluations using industry-standard simulators and emulators.

## 🗂️ Table of Contents
1. [Project 1: Socket Programming (FTP & Chat Server)](#project-1-socket-programming-ftp--chat-server)
2. [Project 2: Wi-Fi Network Performance Evaluation (ns-3)](#project-2-wi-fi-network-performance-evaluation-ns-3)
3. [Project 3: Network Topology & Routing Simulation (GNS3)](#project-3-network-topology--routing-simulation-gns3)
4. [Project 4: Mini-TCP Implementation over UDP](#project-4-mini-tcp-implementation-over-udp)

---

## 💻 Project 1: Socket Programming (FTP & Chat Server)

**Folder:** `01-Socket-Programming`

This project focuses on client-server architecture by implementing a simplified FTP protocol integrated into a group chat system. The communication between the server and clients is established over TCP.

*   **Multi-Client Architecture:** The server is designed to handle multiple clients simultaneously using `fork` (or alternatively threads/select).
*   **Supported Commands:**
    *   `MSG`: Sends a message to the group chat.
    *   `PM`: Sends a private message to a specific user.
    *   `USERS`: Displays a list of all currently online users.
    *   `LIST`: Displays available files on the server.
    *   `GET`: Downloads a specified file.
    *   `PUT`: Uploads a file to the server.
    *   `QUIT`: Safely disconnects the client from the system.
*   **Tech Stack:** C++, TCP Sockets, POSIX Threads/Processes.

---

## 📡 Project 2: Wi-Fi Network Performance Evaluation (ns-3)

**Folder:** `02-WiFi-Performance-ns3`
  
The objective of this project is to simulate and evaluate the performance of Wi-Fi 6 (IEEE 802.11ax) in comparison to Wi-Fi 5 (IEEE 802.11ac) using the ns-3 network simulator.

*   **Topology:** Implemented a Star Topology where multiple stations (STAs) connect to a central Access Point (AP).
*   **Performance Metrics:** Evaluated network efficiency by measuring Throughput, Delay, Packet Loss, and Jain's Fairness Index across different scenarios.
*   **Advanced Features Simulated:**
    *   Integration of OFDMA and MU-MIMO technologies.
    *   Implementation and analysis of UORA (Uplink OFDMA Random Access) and BSRP (Buffer Status Report Polling).
*   **Tech Stack:** ns-3, C++, Python, Linux.
* 
---

## 🛠️ Project 3: Network Topology & Routing Simulation (GNS3)

**Folder:** `03-GNS3-Routing-VLAN`
This project involves the practical setup of network topologies and routing mechanisms using the GNS3 emulator and Cisco IOS images (c7200-adventerprisek9-mz).

*   **VLAN Configuration:** Segmented the network by configuring multiple VLANs (VLAN 10 and VLAN 20) and verified Inter-VLAN communication.
*   **Static Routing:** Manually populated forwarding tables to establish connections across different subnets and tested network redundancy by disabling links.
*   **Dynamic Routing:** Configured the OSPF (Open Shortest Path First) protocol to dynamically build routing tables and discover network neighbors.
*   **Tech Stack:** GNS3, Cisco IOS, Wireshark (for packet analysis).

---

## 📦 Project 4: Mini-TCP Implementation over UDP

**Folder:** `04-Mini-TCP-over-UDP`

This project involves building "Mini-TCP", an educational, application-layer implementation of TCP's core reliability and congestion control features on top of unreliable UDP sockets.

*   **Reliable Data Transfer:** Designed custom packet headers (including sequence numbers, acknowledgment numbers, and checksums) to ensure data integrity.
*   **Flow & Error Control:** Progressed from a basic Stop-and-Wait protocol to a Sliding Window mechanism utilizing cumulative ACKs and timeouts for retransmissions.
*   **Congestion Control:** Implemented core TCP congestion control algorithms, specifically Slow Start and Congestion Avoidance, by dynamically adjusting the Congestion Window (`cwnd`) based on simulated network loss and delay.
*   **Bonus Features:** Enhanced the protocol with Fast Retransmit (triggering retransmission upon receiving three duplicate ACKs) and Fast Recovery.
*   **Tech Stack:** C++, UDP Sockets, Makefile, Python (for plotting `cwnd` and throughput graphs).

---

  
## 🚀 Getting Started

To explore any of these projects, navigate to their respective directories. Detailed instructions on compiling and running the code (e.g., using `make` or ns-3 build scripts) can be found in the project reports.
