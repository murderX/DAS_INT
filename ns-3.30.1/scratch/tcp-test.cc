/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpApplicationExample");

int
main (int argc, char *argv[])
{
  int tcpSegmentSize = 1300;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue(87380));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(87380));

  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::US);
  LogComponentEnable ("TcpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("TcpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3);

  MyNetDeviceHelper  pointToPoint;
  // PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("0.001ms"));

  NetDeviceContainer devices, n1n2;
  devices = pointToPoint.Install (nodes.Get(0), nodes.Get(1));
  n1n2 = pointToPoint.Install (nodes.Get(1), nodes.Get(2));


  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (n1n2);

  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = address.Assign (devices);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  TcpEchoServerHelper echoServer (interfaces.GetAddress (1), 9);
  echoServer.SetAttribute ("EnableINT", BooleanValue(true));

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  TcpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (60));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.000001)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (2048));
  echoClient.SetAttribute ("EnableINT", BooleanValue(true));
  echoClient.SetAttribute ("Deadline", UintegerValue (100000));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowHelper;
  flowmon = flowHelper.InstallAll();

  Simulator::Stop(Seconds(10.0));
  Simulator::Run ();
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("test.xml", true, true);
  Simulator::Destroy ();

  return 0;
}
