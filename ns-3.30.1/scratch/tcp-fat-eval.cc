#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/MyNetDeviceHelper.h"
#include "ns3/tcp-echo-helper.h"
#include "ns3/random-variable-stream.h"

#include <string>


using namespace ns3;
using namespace std;

enum running_mode{
    INT = 0,
    EDF,
    BASE
};

char * toString(int a,int b, int c, int d){

	int first = a;
	int second = b;
	int third = c;
	int fourth = d;

	char *address =  new char[30];
	char firstOctet[30], secondOctet[30], thirdOctet[30], fourthOctet[30];	
	//address = firstOctet.secondOctet.thirdOctet.fourthOctet;

	bzero(address,30);

	snprintf(firstOctet,10,"%d",first);
	strcat(firstOctet,".");
	snprintf(secondOctet,10,"%d",second);
	strcat(secondOctet,".");
	snprintf(thirdOctet,10,"%d",third);
	strcat(thirdOctet,".");
	snprintf(fourthOctet,10,"%d",fourth);

	strcat(thirdOctet,fourthOctet);
	strcat(secondOctet,thirdOctet);
	strcat(firstOctet,secondOctet);
	strcat(address,firstOctet);

	return address;
}

void split(const std::string& s,
    std::vector<std::string>& sv,
                  const char* delim = " ") {
    sv.clear();                                 // 1.
    char* buffer = new char[s.size() + 1];      // 2.
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);      // 3.
    char* p = std::strtok(buffer, delim);       // 4.
    do {
        sv.push_back(p);                        // 5.
    } while ((p = std::strtok(NULL, delim)));   // 6.
    delete[] buffer;
    return;
}

NS_LOG_COMPONENT_DEFINE("INTSchedulingEval");

int main(int argc, char* argv[]) {
    Time::SetResolution (Time::US);
    int tcpSegmentSize = 1300;
    int running_mode = 2; // 0 for INT, 1 to EDF, 2 to use baseline
    int k = 8;
    int numPerRack = 25;
    int clientNum = 300;
    int ratio_of_back = 0;
    string stat_name("stat/INT-eval.xml");
    string config_name("config.txt");   // Place holder to improve the efficiency

    CommandLine cmd;
    cmd.AddValue("eval", "0 for INT, 1 to use EDF, 2 to perform baseline", running_mode);
    cmd.AddValue("o", "Output xml file name", stat_name);
    cmd.AddValue("seg", "tcp segment size", tcpSegmentSize);
    cmd.AddValue("k", "k value for fat tree", k);
    cmd.AddValue("rack", "number of machines per rack", numPerRack);
    cmd.AddValue("cnum", "client num to adjust link utilization", clientNum);
    cmd.AddValue("ratio", "ratio of the non-deadline flow in INT mode", ratio_of_back);
    cmd.AddValue("config", "config file for the eval", config_name);
    cmd.Parse(argc, argv);

    // Setup Configuration
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));
    Config::SetDefault("ns3::TcpSocketBase::Alpha", DoubleValue(1.0/8));
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId())); 

    string mode_str;
    switch (running_mode)
    {
    case INT:
        mode_str = "INT";
        break;
    case EDF:
        mode_str = "EDF";
        break;
    case BASE:
    default:
        mode_str = "Baseline";
        break;
    }

    // Connecting topology
    int total = k * k / 4 + k * k + k * k / 2 * numPerRack;
    NodeContainer allNode;
    allNode.Create(total);
    NodeContainer core;
    NodeContainer agg[k];
    NodeContainer edge[k];
    NodeContainer client[k][k/2];

    int current = 0;
    // Adding core
    for(int i=0; i< k * k / 4; i++)
    {
        core.Add(allNode.Get(current));
        current++;
    }
    // Add Agg
    for(int i=0; i<k; i++)
    {
        for(int j=0;j<k/2; j++)
        {
            agg[i].Add(allNode.Get(current));
            current++;
        }
    }
    //Add Edge
    for(int i=0; i<k; i++)
    {
        for(int j=0;j<k/2; j++)
        {
            edge[i].Add(allNode.Get(current));
            current++;
        }
    }

    // Add Client
    for(int i=0; i < k;i++)
    {
        for(int j=0;j<k/2;j++)
        {
            for(int z=0;z<numPerRack;z++)
            {
                client[i][j].Add(allNode.Get(current));
                current++;
            }
        }

    }


    // Building Connection
    InternetStackHelper stack;
	stack.Install(allNode);

    // Setting up link properties
    // INT helper
    MyNetDeviceHelper p2p, p2pBot;
    p2p.SetDeviceAttribute("DataRate", StringValue ("4Gbps"));
    p2pBot.SetDeviceAttribute("DataRate", StringValue ("1Gbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("0.001ms"));
    p2pBot.SetChannelAttribute ("Delay", StringValue ("0.001ms"));

    // Baseline Helper
    PointToPointHelper point2point, point2pointBot;
    point2point.SetDeviceAttribute("DataRate", StringValue ("4Gbps"));
    point2pointBot.SetDeviceAttribute("DataRate", StringValue ("1Gbps"));
    point2point.SetChannelAttribute ("Delay", StringValue ("0.001ms"));
    point2pointBot.SetChannelAttribute ("Delay", StringValue ("0.001ms"));

    if(running_mode == 1) {
        point2point.SetEDF(true);
        point2pointBot.SetEDF(true);
    }

    std::cout<<"Connecting Topology with k="<<k<<std::endl;
    // Connecting Core to Agg
    Ipv4AddressHelper address;

    NetDeviceContainer core2agg[k * k / 4][k];
    for(int i=0; i < k * k / 4; i++) {
        int tar_agg = i * 2 / k;
        for(int j = 0; j < k; j++) {
            if(running_mode == 0) {
               core2agg[i][j] = p2p.Install(NodeContainer(core.Get(i), agg[j].Get(tar_agg)));
            }
            else {
               core2agg[i][j] = point2point.Install(NodeContainer(core.Get(i), agg[j].Get(tar_agg)));
            }
            char *base=toString(10, i, j, 0);
            address.SetBase(base, "255.255.255.0");
            address.Assign(core2agg[i][j]);
        }
    }

    //Connect Agg to Edge
    NetDeviceContainer agg2edge[k][k/2][k / 2];
    for(int i=0; i < k; i++)
    {
        for(int j =0; j < k/2; j++)
        {
            for(int z=0; z < k / 2; z++)
            {
                if(running_mode == 0)
                {
                    agg2edge[i][j][z] = p2p.Install(NodeContainer(agg[i].Get(j), edge[i].Get(z)));
                }
                else
                    agg2edge[i][j][z] = point2point.Install(NodeContainer(agg[i].Get(j), edge[i].Get(z)));
                char *base = toString(20, i * k / 2 + j, z, 0);
                address.SetBase(base, "255.255.255.0");
                address.Assign(agg2edge[i][j][z]);
            }
        }
    }

    // Connect Edge to host
    NetDeviceContainer edge2host[k][k/2][numPerRack];
    Ipv4InterfaceContainer hostNIC[k][k/2][numPerRack];
    for(int i=0; i < k; i++) {
        for(int j=0; j < k / 2; j++) {
            for(int z = 0; z < numPerRack; z++) {
                if(running_mode == 0) {
                    edge2host[i][j][z] = p2pBot.Install(NodeContainer(edge[i].Get(j), client[i][j].Get(z)));
                }
                else {
                    edge2host[i][j][z] = point2pointBot.Install(NodeContainer(edge[i].Get(j), client[i][j].Get(z)));
                }
                char *base = toString(100+i, j, z, 0);
                address.SetBase(base,"255.255.255.0");
                hostNIC[i][j][z] = address.Assign(edge2host[i][j][z]);
            }
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    std::cout<<"Finished Connecting Topology..."<<std::endl;

    std::cout<<"Installing "<<clientNum<<" Servers&Clients..."<<std::endl;
    // for(int i = 0; i < k; i++){
    //     for(int j = 0; j < k/2; j++){
    //         for(int z = 0; z < numPerRack; z++){
    //             ApplicationContainer server;
    //             if(running_mode == 1) {
    //                 TcpEchoServerHelper echoServer(hostNIC[i][j][z].GetAddress(1), 6233);
    //                 echoServer.SetAttribute("EnableINT", BooleanValue(true));
    //                 server = echoServer.Install(client[i][j].Get(z));
    //             }
    //             else if(running_mode == 2) {
    //                 PacketSinkHelper sinkServer("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 6233));
    //                 server = sinkServer.Install(client[i][j].Get(z));
    //             }
    //             else if(running_mode == 0) {

    //             }
    //             server.Start(Seconds(0.1));
    //             server.Stop(Seconds(30.0));
    //         }
    //     }
    // }

    int marker[k][k/2][numPerRack];
    Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable>();
    Ptr<ExponentialRandomVariable> expGen = CreateObject<ExponentialRandomVariable>();
    expGen->SetAttribute("Mean", DoubleValue(3));
    Ptr<ExponentialRandomVariable> expTimeGen = CreateObject<ExponentialRandomVariable>();
    expTimeGen->SetAttribute("Mean", DoubleValue(3.0));
    for(int j = 0; j<clientNum; j++) {
        // Host location
        int pod = generator->GetInteger(0, k - 1);
        int rack  = generator->GetInteger(0, k/2 - 1);
        int host = generator->GetInteger(0, numPerRack - 1);
        // Installing host
        if(marker[pod][rack][host] != 1) {
            ApplicationContainer server;
            TcpEchoServerHelper echoServer(hostNIC[pod][rack][host].GetAddress(1), 6233);
            if(running_mode == 0) {
                echoServer.SetAttribute("EnableINT", BooleanValue(true));
            }
            server = echoServer.Install(client[pod][rack].Get(host));
            server.Start(Seconds(0.1));
            server.Stop(Seconds(30.0));
            marker[pod][rack][host] = 1;
        }
        
        TcpEchoClientHelper echoClient(hostNIC[pod][rack][host].GetAddress(1), 6233);
        int flowSize = generator->GetInteger(2, 198);;
        int useDDL = generator->GetInteger(1, 10);
        if(useDDL > ratio_of_back && running_mode != 2) {
            double val = expGen->GetValue();
            int ddl = val > 0.5 ? (int)(val*1000):500;
            if(running_mode == 0) {
                echoClient.SetAttribute("EnableINT", BooleanValue(true));
            }
            else if(running_mode == 1) {
                echoClient.SetAttribute("UseEDF", BooleanValue(true));
            }
            echoClient.SetAttribute("Deadline", UintegerValue(ddl));
        }
        echoClient.SetAttribute("MaxPackets", UintegerValue(flowSize / 2));
        echoClient.SetAttribute("PacketSize", UintegerValue(2048));
        echoClient.SetAttribute("Interval", TimeValue (MicroSeconds(10)));

        // Client Location
        int client_pod = generator->GetInteger(0, k - 1);
        int client_rack = generator->GetInteger(0, k/2 - 1);
        int client_host = generator->GetInteger(0, numPerRack - 1);

        echoClient.SetAttribute("LocalAddress", Ipv4AddressValue(hostNIC[client_pod][client_rack][client_host].GetAddress(1)));
        ApplicationContainer clientApps = echoClient.Install(client[client_pod][client_rack].Get(client_host));
        double start_time = expTimeGen->GetValue();
        clientApps.Start(Seconds(start_time > 0.1 ? start_time: 0.1));
        clientApps.Stop(Seconds(20));
    }

    std::cout<<"Installation finished. Starting simulation in mode "<<mode_str<<std::endl;
    if(running_mode != 2)
    std::cout<<"If_finished\tSrc\tDst\tFCT\tDeadline\tFlow size/KB"<<std::endl;
    else {
        std::cout<<"If_finished\tSrc\tDst\tFCT\tFlow size/KB"<<std::endl;
    }

    FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop (Seconds(31.0));
    Simulator::Run ();
    monitor->CheckForLostPackets ();
  	monitor->SerializeToXmlFile(stat_name, true, true);
    Simulator::Destroy ();
    std::cout<<"Finished Simulation!"<<std::endl;
}
