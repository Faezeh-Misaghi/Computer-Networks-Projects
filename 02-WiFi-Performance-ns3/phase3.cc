#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wifi6_UORA_Phase3");

static std::ofstream g_sinrFile;

//--- SINR trace callback ---
void SinrTrace(std::string context,
               Ptr<const Packet> packet,
               uint16_t channelFreqMhz,
               WifiTxVector txVector,
               MpduInfo mpduInfo,
               SignalNoiseDbm signalNoiseDbm,
               uint16_t staId)
{
    double snr = signalNoiseDbm.signal - signalNoiseDbm.noise;

    g_sinrFile << Simulator::Now().GetSeconds() << ","
               << context << ","
               << channelFreqMhz << ","
               << staId << ","
               << signalNoiseDbm.signal << ","
               << signalNoiseDbm.noise << ","
               << snr << ","
               << packet->GetSize() << "\n";
}

int main (int argc, char *argv[])
{
    //--- simulation parameters ---
    uint32_t nStas = 5;
    double duration = 10.0;

    CommandLine cmd;
    cmd.AddValue ("nStas", "Number of stations", nStas);
    cmd.Parse (argc, argv);

    //--- output files ---
    g_sinrFile.open ("sinr_output.csv");
    g_sinrFile << "Time,Context,FreqMHz,StaId,SignalDbm,NoiseDbm,SNRdB,PacketSize\n";

    //--- nodes (AP + STAs) ---
    NodeContainer staNodes;
    staNodes.Create (nStas);

    NodeContainer apNode;
    apNode.Create (1);

    //--- channel / PHY (spectrum) ---
    SpectrumWifiPhyHelper phy;
    Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
    phy.SetChannel (channel);
    phy.Set ("TxPowerStart", DoubleValue (16.0));
    phy.Set ("TxPowerEnd", DoubleValue (16.0));

    //--- Wi-Fi helper (802.11ax + rate control) ---
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211ax);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue ("HeMcs0"),
                                  "ControlMode", StringValue ("HeMcs0"));

    //--- MAC configuration (SSID, AP MAC, STA MAC) ---
    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns3-uora");

    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));
    NetDeviceContainer apDevice = wifi.Install (phy, mac, apNode);

    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid));
    NetDeviceContainer staDevices = wifi.Install (phy, mac, staNodes);

    //--- mobility (star-like random placement + fixed AP position) ---
    MobilityHelper mobility;

    // STA positions: uniform random in a disc of radius 50m around (0,0)
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "rho", DoubleValue (50.0),
                                   "X", DoubleValue (0.0),
                                   "Y", DoubleValue (0.0));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (staNodes);

    // AP position: fixed at (0,0,1.5)
    Ptr<ListPositionAllocator> apPosition = CreateObject<ListPositionAllocator> ();
    apPosition->Add (Vector (0.0, 0.0, 1.5));
    mobility.SetPositionAllocator (apPosition);
    mobility.Install (apNode);

    //--- internet stack ---
    InternetStackHelper stack;
    stack.Install (apNode);
    stack.Install (staNodes);

    //--- IP addressing ---
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces = address.Assign (staDevices);
    Ipv4InterfaceContainer apInterface = address.Assign (apDevice);

    //--- applications (UL UDP clients on STAs -> UDP sink on AP) ---
    uint16_t port = 9;

    PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory",
                                 InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApp = sinkHelper.Install (apNode.Get (0));
    sinkApp.Start (Seconds (0.0));
    sinkApp.Stop (Seconds (duration));

    Ptr<UniformRandomVariable> randomStart = CreateObject<UniformRandomVariable> ();

    for (uint32_t i = 0; i < nStas; ++i)
    {
        UdpClientHelper client (apInterface.GetAddress (0), port);
        client.SetAttribute ("MaxPackets", UintegerValue (4294967295U));
        client.SetAttribute ("Interval", TimeValue (MilliSeconds (50)));
        client.SetAttribute ("PacketSize", UintegerValue (93));

        ApplicationContainer clientApp = client.Install (staNodes.Get (i));
        clientApp.Start (Seconds (randomStart->GetValue (0.1, 0.5)));
        clientApp.Stop (Seconds (duration));
    }

    //--- PHY tracing (MonitorSnifferRx -> write SNR/SINR-related fields to CSV) ---
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx",
                     MakeCallback (&SinrTrace));

    //--- FlowMonitor (per-flow stats) ---
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    //--- run simulation ---
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();

    //--- export FlowMonitor results (XML + console summary) ---
    monitor->CheckForLostPackets ();
    monitor->SerializeToXmlFile ("phase3_results.xml", true, true);

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    double totalThroughput = 0.0;

    for (const auto& flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (flow.first);

        double throughputKbps = flow.second.rxBytes * 8.0 / (duration * 1000.0);

        double meanDelay = 0.0;
        if (flow.second.rxPackets > 0)
        {
            meanDelay = flow.second.delaySum.GetSeconds () / flow.second.rxPackets;
        }

        double plr = 0.0;
        if (flow.second.txPackets > 0)
        {
            plr = static_cast<double> (flow.second.lostPackets) / flow.second.txPackets;
        }

        std::cout << "Flow " << flow.first
                  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Throughput: " << throughputKbps << " Kbps\n";
        std::cout << "  Mean Delay: " << meanDelay << " s\n";
        std::cout << "  Packet Loss Ratio: " << plr << "\n";

        totalThroughput += throughputKbps;
    }

    std::cout << "Total Network Throughput: " << totalThroughput << " Kbps" << std::endl;

    //--- cleanup ---
    g_sinrFile.close ();
    Simulator::Destroy ();
    return 0;
}
