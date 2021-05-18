#ifndef MY_DEVICE_HELPER_H
#define MY_DEVICE_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/MyNetDevice.h"

#include "ns3/trace-helper.h"
#include "ns3/point-to-point-helper.h"

namespace ns3 {

class NetDevice;
class Node;

class MyNetDeviceHelper:  public PointToPointHelper
{
    public:
    MyNetDeviceHelper();
    virtual ~MyNetDeviceHelper();
    NetDeviceContainer  Install (Ptr<Node> a, Ptr<Node> b);
    NetDeviceContainer Install (NodeContainer c);
    void SetDeviceAttribute (std::string n1, const AttributeValue &v1);
    void SetChannelAttribute (std::string n1, const AttributeValue &v1);

  /**
   * \param a first node
   * \param bName name of second node
   * \return a NetDeviceContainer for nodes
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
    NetDeviceContainer Install (Ptr<Node> a, std::string bName);

  /**
   * \param aName Name of first node
   * \param b second node
   * \return a NetDeviceContainer for nodes
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
    NetDeviceContainer Install (std::string aName, Ptr<Node> b);

  /**
   * \param aNode Name of first node
   * \param bNode Name of second node
   * \return a NetDeviceContainer for nodes
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
    NetDeviceContainer Install (std::string aNode, std::string bNode);
    private:
  /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);
  

  /**
   * \brief Enable ascii trace output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename);

  ObjectFactory m_queueFactory;         //!< Queue Factory
  ObjectFactory m_channelFactory;       //!< Channel Factory
  ObjectFactory m_remoteChannelFactory; //!< Remote Channel Factory
  ObjectFactory m_deviceFactory;        //!< Device Factory
};

}

#endif