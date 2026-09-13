#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiProjectPhase2");

int main (int argc, char *argv[])
{

    uint32_t station_num = 5;
    NodeContainer sta_nodes;
    sta_nodes.Create (station_num);
    NodeContainer ap_node;
    ap_node.Create (1);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy_layer;
    phy_layer.SetChannel(channel.Create());
    // phy_layer.Set ("RxNoiseFigure", DoubleValue (15.0));
    // phy_layer.Set ("RxNoiseFigure", DoubleValue (15.5));
    phy_layer.Set ("RxNoiseFigure", DoubleValue (20.0));

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211ax);

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HeMcs4"),"ControlMode", StringValue ("HeMcs0"));

    WifiMacHelper mac;
    Ssid ssid = Ssid("my-wifi6-network");

    mac.SetType("ns3::StaWifiMac","Ssid", SsidValue(ssid));
    NetDeviceContainer sta_devices = wifi.Install(phy_layer, mac, sta_nodes);

    mac.SetType("ns3::ApWifiMac","Ssid", SsidValue(ssid));
    NetDeviceContainer ap_device = wifi.Install(phy_layer, mac, ap_node);

    Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator> ();
    positions->Add (Vector (0.0, 0.0, 0.0));
    double radius = 5.0; 
    double angleStep = 2 * M_PI / station_num;
    for (uint32_t i = 0; i < station_num; ++i)
    {
        double angle = i * angleStep;
        double x = radius * cos (angle);
        double y = radius * sin (angle);
        positions->Add (Vector (x, y, 0.0));
    }

    MobilityHelper mobility;
    mobility.SetPositionAllocator (positions);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ap_node);
    mobility.Install (sta_nodes);

    InternetStackHelper stack;
    stack.Install (ap_node);
    stack.Install (sta_nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ap_interface = address.Assign (ap_device);
    Ipv4InterfaceContainer sta_interfaces = address.Assign (sta_devices);

    uint16_t port = 9; 
    UdpEchoServerHelper echo_server (port);

    ApplicationContainer server_apps = echo_server.Install (ap_node.Get (0));
    server_apps.Start (Seconds (0.0));
    server_apps.Stop (Seconds (10.0));

    UdpEchoClientHelper echo_client (ap_interface.GetAddress (0), port);
    echo_client.SetAttribute ("MaxPackets", UintegerValue (20)); 
    echo_client.SetAttribute ("Interval", TimeValue (Seconds (0.5)));

    ApplicationContainer client_apps;

    for (uint32_t i = 0; i < sta_nodes.GetN(); ++i)
    {
        if (i == 0 || i == 2 || i == 4){
            echo_client.SetAttribute ("PacketSize", UintegerValue (1024));
        }else{
            echo_client.SetAttribute ("PacketSize", UintegerValue (512));
        }
        client_apps.Add(echo_client.Install (sta_nodes.Get (i)));
    }

    client_apps.Start (Seconds (0.0));
    client_apps.Stop (Seconds (10.0));

    Simulator::Stop (Seconds (10.0));

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Run ();

    monitor->CheckForLostPackets();
    // monitor->SerializeToXmlFile("flowmon_wifi6_2.xml", true, true);
    // monitor->SerializeToXmlFile("flowmon_wifi6_3.xml", true, true);
    monitor->SerializeToXmlFile("flowmon_wifi6_4.xml", true, true);

    Simulator::Destroy ();
    return 0;
}
