/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/int-header.h"
#include "tcp-echo-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpEchoServerApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpEchoServer);

TypeId
TcpEchoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpEchoServer")
    .SetParent<Application> ()
    .AddConstructor<TcpEchoServer> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&TcpEchoServer::m_local),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&TcpEchoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("EnableINT",
                   "To enable INT for the traffic",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TcpEchoServer::m_int_das),
                   MakeBooleanChecker ())
  ;
  return tid;
}

TcpEchoServer::TcpEchoServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

TcpEchoServer::~TcpEchoServer()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void
TcpEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void 
TcpEchoServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
	{
		TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		if(m_int_das) {
			m_socket->SetAttribute("EnableINT", BooleanValue(true));
			m_socket->SetAttribute("INT_CSMode", BooleanValue(true));
		}
		InetSocketAddress local = InetSocketAddress (m_local, m_port);
		int res = m_socket->Bind (local);
                (void)res;
		m_socket->Listen();
		// NS_LOG_INFO("Echo Server local address:  " << m_local << " port: " << m_port << " bind: " << res );
	}

  m_socket->SetRecvCallback (MakeCallback (&TcpEchoServer::HandleRead, this));
  m_socket->SetAcceptCallback (
    //MakeCallback (&TcpEchoServer::HandleAcceptRequest, this),
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&TcpEchoServer::HandleAccept, this));
	// m_socket->SetCloseCallbacks(MakeCallback(&TcpEchoServer::HandleClose, this), MakeCallback(&TcpEchoServer::HandleClose, this));
}

void TcpEchoServer::HandleClose(Ptr<Socket> s1)
{
	NS_LOG_INFO(" PEER CLOSE ");
	NS_LOG_INFO("**********************************************************");
        s1->Close();

	// Ptr<Socket> s2 = m_pair[s1];

	// m_conn.erase(s1);
	// m_conn.erase(s2);
	// m_pair.erase(s1);
	// m_pair.erase(s2);	
	// PrintPairs();
	// NS_LOG_INFO("Closing socket S1 " << s1);
	// if(s1) {
	// 	s1->Close();
	// }
	// NS_LOG_INFO("Closing socket S2 " << s2);
	// s2->Close();
	// NS_LOG_INFO("Done closing sockets " << s1 << " " << s2);
	// if (s1->m_pair) {
	// 	s1->m_pair->Close();
	// 	s1->m_pair->m_pair = NULL;
	// 	s1->m_pair = NULL;
	// }
}

bool TcpEchoServer::HandleAcceptRequest (Ptr<Socket> s, const Address& from)
{
	// NS_LOG_INFO(" HANDLE ACCEPT REQUEST FROM " <<  InetSocketAddress::ConvertFrom(from));
	return true;
}

void TcpEchoServer::HandleAccept (Ptr<Socket> s, const Address& from)
{
	NS_LOG_FUNCTION (this << s << from);
	// NS_LOG_INFO("ACCEPT IN ECHO SERVER from " << InetSocketAddress::ConvertFrom(from).GetIpv4());
	s->SetRecvCallback (MakeCallback (&TcpEchoServer::HandleRead, this));
        s->SetCloseCallbacks(MakeCallback(&TcpEchoServer::HandleClose, this), MakeCallback (&TcpEchoServer::HandleClose, this));       

	// NS_LOG_INFO("FROM: " << InetSocketAddress::ConvertFrom(from).GetIpv4());
	m_conn[s] = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
}

void 
TcpEchoServer::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void TcpEchoServer::PrintPairs() {
	std::map<Ptr<Socket>, Ptr<Socket> >::iterator pi;
	NS_LOG_INFO("**********************************************************");
	for (pi = m_pair.begin(); pi != m_pair.end(); pi++) {
		NS_LOG_INFO(" [ " << (*pi).first << " ] -> [ " << (*pi).second << " ]");
	}
	std::map<Ptr<Socket>, Ipv4Address>::iterator ci;
	NS_LOG_INFO("**********************************************************");
	for (ci = m_conn.begin(); ci != m_conn.end(); ci++) {
		NS_LOG_INFO(" [ " << (*ci).first << " ] -> [ " << (*ci).second << " ]");
	}

}

void 
TcpEchoServer::HandleRead (Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	Ptr<Packet> packet2;
	Address from;
	while ( (packet = socket->RecvFrom (from)) )
	{
		uint8_t *msg;
		msg = new uint8_t[packet->GetSize()];
		packet->CopyData(msg, packet->GetSize());
        Ipv4Address dst;
        dst = InetSocketAddress::ConvertFrom (from).GetIpv4 ();

		/*NS_LOG_INFO (m_local << " received " << packet->GetSize () << " bytes from " <<
				InetSocketAddress::ConvertFrom (from).GetIpv4 ());
		NS_LOG_INFO ("PACKET ID: " << packet->GetUid() << "====> CONTENT:" << msg << " SIZE: " << packet->GetSize());*/


		if (m_conn.find(socket) == m_conn.end()) {
			NS_LOG_INFO("WARNING: Received packet from unknown socket " << socket);
			return;
		}

		if (InetSocketAddress::IsMatchingType (from))
		{
			// NS_LOG_INFO (m_local << " received " << packet->GetSize () << " bytes from " <<
			// 		InetSocketAddress::ConvertFrom (from).GetIpv4 ());

            if (!m_int_das) {
			   packet2 = Create<Packet> (msg, packet->GetSize());
			   socket -> Send (packet2);
                           // NS_LOG_INFO("SENDING the packet to destination: " << dst);
			}
			
			// NS_LOG_INFO("orig socket " << socket << " dest socket " << m_pair[socket]);
		}
		delete msg;
	}
}

} // Namespace ns3
