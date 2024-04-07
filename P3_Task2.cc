/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <iostream>
//
// 
//
// *
// |     10.1.1.0  Router
// n0 -------------- n1 -------------- n2
//                        10.1.2.0		|
//                                      *  		
//                                      

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpIpSimulation");


void RunSimulation(Ptr<Ipv4FlowClassifier> classifier, Ptr<FlowMonitor> monitor, double startTime, std::ofstream& outFd, double errorRate) {

	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    
    for (auto iter : stats) {
			
		auto fid = iter.first;
		auto fstats = iter.second;
		auto t = classifier->FindFlow(fid);
		std::cout << "Flow ID: " << fid << " Packet Loss Rate " << errorRate << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<< std::endl;
		std::cout << "Start Time = " << startTime << std::endl;
		std::cout << "Tx Packets = " << fstats.txPackets << std::endl;
		std::cout<< "Rx Packets = " << fstats.rxPackets << std::endl;
		
		double throughput = fstats.rxBytes * 8.0 / (fstats.timeLastRxPacket.GetSeconds() - fstats.timeFirstTxPacket.GetSeconds()) / 1024 / 1024;
		
		std::cout<< "Throughput: " << throughput << " Mbps" << std::endl;
		if (fid == 1 && fstats.rxPackets > 0)
		{
		    //double latency = (fstats.timeFirstRxPacket.GetSeconds() - fstats.timeFirstTxPacket.GetSeconds()) * 1000 ;
		    double latency = fstats.delaySum.GetSeconds() / fstats.rxPackets * 1000;
		    std::cout<< "End-to-end Latency: " << latency << " ms" << std::endl;
		    outFd << errorRate <<"," << startTime <<","<< fid << "," << t.sourceAddress << "," << t.destinationAddress;
			outFd << "," << fstats.txPackets << "," << fstats.rxPackets;
			outFd << "," << throughput << "," << latency << "\n";
		}
		
	}
}

int
main(int argc, char* argv[])
{

	Time::SetResolution(Time::NS);
    bool verbose = true;
    uint32_t nDevices = 3;
    double warmUpTime = 5.0, simulationTime = 110;
    bool tracing = true;

    CommandLine cmd(__FILE__);
    //cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    //cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);
  
    NodeContainer nodes;
    nodes.Create(nDevices);
    
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));
    
    
    NetDeviceContainer device1ToRouterDevices = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer routerToDevice2Devices = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    
    InternetStackHelper stack;
    stack.Install(nodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer device1ToRouterInterfaces = address.Assign(device1ToRouterDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer routerToDevice2Interfaces = address.Assign(routerToDevice2Devices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    uint16_t port = 9;
    BulkSendHelper source("ns3::TcpSocketFactory",
                          InetSocketAddress(routerToDevice2Interfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Seconds(warmUpTime + 1.0));
    sourceApps.Stop(Seconds(simulationTime));
    
    PacketSinkHelper sink("ns3::TcpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(2));
    sinkApps.Start(Seconds(warmUpTime + 1.0));
    sinkApps.Stop(Seconds(simulationTime));
    
	FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simulationTime));

    if (tracing)
    {
        //pointToPoint.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        //pointToPoint.EnablePcapAll("third");
        pointToPoint.EnablePcapAll("tcp-ip-simulation");
        //pointToPoint.EnablePcap("third_task3_part1_pcap_ap1", apDevices1.Get(0));
        //pointToPoint.EnablePcap("third_task3_part1_pcap_ap2", apDevices2.Get(0));
        //csma.EnablePcap("third", csmaDevices.Get(0), true);
    }
    //AnimationInterface anim("P3_Simulation_model.xml");
    
    std::ofstream outFd;
    outFd.open("simulation_results.csv", std::ofstream::app);
    //outFd << "FlowID,SourceAddress,DestinationAddress,TxPackets,RxPackets,Throughput,Latency\n";
    
    double errorRate = std::stod(argv[1]);
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
    em->SetAttribute("ErrorRate", DoubleValue(errorRate));
    device1ToRouterDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
	routerToDevice2Devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    
	
	for(double t=5.0; t<110.0; t+=10.0) {
		monitor->CheckForLostPackets();
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
		//std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
		Simulator::Schedule(Seconds(t), [classifier, monitor, t, &outFd, errorRate] {
        RunSimulation(classifier, monitor, t, outFd, errorRate);
		});
	}
		Simulator::Run();
	outFd.close();
    
    Simulator::Destroy();
    return 0;
}
