#include <iostream>

#include "myRemoteChannel.h"
#include "MyNetDevice.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mpi-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyRemoteChannel");

NS_OBJECT_ENSURE_REGISTERED (MyRemoteChannel);

TypeId
MyRemoteChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyRemoteChannel")
    .SetParent<MyChannel> ()
    .SetGroupName ("Applications")
    .AddConstructor<MyRemoteChannel> ()
  ;
  return tid;
}

MyRemoteChannel::MyRemoteChannel ()
  : MyChannel ()
{
}

MyRemoteChannel::~MyRemoteChannel ()
{
}

bool
MyRemoteChannel::TransmitStart (
  Ptr<const Packet> p,
  Ptr<MyNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  IsInitialized ();

  uint32_t wire = src == GetSource (0) ? 0 : 1;
  Ptr<MyNetDevice> dst = GetDestination (wire);

#ifdef NS3_MPI
  // Calculate the rxTime (absolute)
  Time rxTime = Simulator::Now () + txTime + GetDelay ();
  MpiInterface::SendPacket (p->Copy (), rxTime, dst->GetNode ()->GetId (), dst->GetIfIndex ());
#else
  NS_FATAL_ERROR ("Can't use distributed simulator without MPI compiled in");
#endif
  return true;
}

} // namespace ns3