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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "tcp-echo-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpEchoClientApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpEchoClient);

TypeId
TcpEchoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpEchoClient")
    .SetParent<Application> ()
    .AddConstructor<TcpEchoClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TcpEchoClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Deadline", 
                   "The deadline of the flow",
                   UintegerValue (0),
                   MakeUintegerAccessor (&TcpEchoClient::m_deadline),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute("EnableINT",
                  "To enable INT for the traffic",
                  BooleanValue (false),
                  MakeBooleanAccessor (&TcpEchoClient::m_int_das),
                  MakeBooleanChecker ())
    .AddAttribute("UseEDF",
                  "To enable INT for the traffic",
                  BooleanValue (false),
                  MakeBooleanAccessor (&TcpEchoClient::m_use_edf),
                  MakeBooleanChecker ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&TcpEchoClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&TcpEchoClient::m_peerAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("LocalAddress", 
                   "The local Ipv4Address of the outbound packets",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&TcpEchoClient::m_localAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&TcpEchoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TcpEchoClient::SetDataSize,
                                         &TcpEchoClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    // .AddTraceSource ("Tx", "A new packet is created and is sent",
    //                  MakeTraceSourceAccessor (&TcpEchoClient::m_txTrace))
  ;
  return tid;
}

TcpEchoClient::TcpEchoClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
  m_deadline = 0;
  m_is_terminated = false;
}

TcpEchoClient::~TcpEchoClient()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
TcpEchoClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

void
TcpEchoClient::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void 
TcpEchoClient::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if(m_int_das) {
        m_socket->SetAttribute("EnableINT", BooleanValue(m_int_das));
        m_socket->SetAttribute("Deadline", UintegerValue(m_deadline));
        m_socket->SetAttribute("INT_CSMode", BooleanValue(false));
      }
      else if(m_use_edf) {
        m_socket->SetAttribute("UseEDF", BooleanValue(m_int_das));
        m_socket->SetAttribute("Deadline", UintegerValue(m_deadline));
      }
      m_socket->SetCloseCallbacks(MakeCallback (&TcpEchoClient::HandleClose, this), MakeCallback (&TcpEchoClient::ErrorClose, this));
      m_socket->Bind ();
      m_start_time = Simulator::Now().GetMicroSeconds();
      m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
    }
  
  if (m_int_das) {
     m_ih.deadline = m_deadline;
     m_ih.mode = 1;
  }

  m_socket->SetRecvCallback (MakeCallback (&TcpEchoClient::HandleRead, this));

  ScheduleTransmit (Seconds (0.));
}

void 
TcpEchoClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void 
TcpEchoClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
TcpEchoClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_size;
}

void 
TcpEchoClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
TcpEchoClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
TcpEchoClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
TcpEchoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sendEvent = Simulator::Schedule (dt, &TcpEchoClient::Send, this);
}

void 
TcpEchoClient::Send (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_sendEvent.IsExpired ());
  if((m_int_das || m_use_edf) && (Simulator::Now().GetMicroSeconds() - m_start_time > m_deadline)) {
    std::cout<<"0\t"<<m_localAddress<<"\t"<<m_peerAddress<<"\t"<<m_deadline<<"\t"<<m_count * m_size / 1024<<std::endl;
    m_is_terminated = true;
    StopApplication();
    return;
  }
  if(m_sent >= m_count) {
     ScheduleTransmit (m_interval);
     return;
  }

  Ptr<Packet> p;
  // TODO(huasunsx): Not correct, socket may buffer the packet, should be solved in TCP socket level. 
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "TcpEchoClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "TcpEchoClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that she doesn't care 
      // about the data itself either by specifying the data size by setting
      // the corresponding atribute or by not calling a SetFill function.  In 
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  int res = m_socket->Send (p);
  if(res != -1) {
     ++m_sent;
     // NS_LOG_INFO ("Sent " << m_size << " bytes to " << m_peerAddress << " and result was: " << res);
  }

  if (m_sent < m_count) 
    {
      ScheduleTransmit (m_interval);
    }
  else {
     if(m_socket->Close () == 0)
     {
       m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
     }
  }
}

void
TcpEchoClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ( (packet = socket->RecvFrom (from)) )
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          // NS_LOG_INFO ("Client Received " << packet->GetSize () << " bytes from " <<
          //             InetSocketAddress::ConvertFrom (from).GetIpv4 ());
        }
    }
}

void
TcpEchoClient::HandleClose (Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION(this<<socket);
    if(!m_is_terminated)
    {
      if(m_int_das || m_use_edf)
        std::cout<<1<<"\t"<<m_localAddress<<"\t"<<m_peerAddress<<"\t"<<Simulator::Now().GetMicroSeconds() - m_start_time <<"\t"<<m_deadline<<"\t"<<m_count * m_size / 1024<<std::endl;
      else
        std::cout<<"2\t"<<m_localAddress<<"\t"<<m_peerAddress<<"\t"<<Simulator::Now().GetMicroSeconds() - m_start_time <<"\t"<<m_count * m_size / 1024<<std::endl;
    }
    m_socket = 0;
    StopApplication();
}

void
TcpEchoClient::ErrorClose (Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION(this<<socket);
    std::cout<<"ErrorClose"<<std::endl;
    StopApplication();
}

void
TcpEchoClient::TraceConnState(TcpSocket::TcpStates_t old, TcpSocket::TcpStates_t new_state)
{
    std::cout<<"Trace Conn State Changed, old state:"<<old<<", new state "<<new_state<<std::endl;
}

} // Namespace ns3
