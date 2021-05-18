#ifndef MY_NET_DEVICE_H
#define MY_NET_DEVICE_H

#include "ns3/point-to-point-net-device.h"
#include "ns3/deadline-queue.h"

namespace ns3
{

class MyChannel;
class ErrorModel;
class DDLQueue;
    class MyNetDevice:public NetDevice
    {
        public:
        static TypeId GetTypeId (void);
        MyNetDevice();
        virtual ~MyNetDevice();
        void Receive (Ptr<Packet> p);
        bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
        void SetReceiveErrorModel (Ptr<ErrorModel> em);
        void SetQueue(Ptr<DDLQueue> q);
        virtual Ptr<Channel> GetChannel (void) const;
        void SetDataRate (DataRate bps);
        void SetInterframeGap (Time t);
        bool Attach (Ptr<MyChannel> ch);
        Ptr<DDLQueue> GetQueue (void) const;
        virtual void SetIfIndex (const uint32_t index);
        virtual uint32_t GetIfIndex (void) const;
        virtual void SetAddress (Address address);
        virtual Address GetAddress (void) const;
        virtual bool SetMtu (const uint16_t mtu);
        virtual uint16_t GetMtu (void) const;

        virtual bool IsLinkUp (void) const;

        virtual void AddLinkChangeCallback (Callback<void> callback);

        virtual bool IsBroadcast (void) const;
        virtual Address GetBroadcast (void) const;

        virtual bool IsMulticast (void) const;
        virtual Address GetMulticast (Ipv4Address multicastGroup) const;

        virtual bool IsPointToPoint (void) const;
        virtual bool IsBridge (void) const;
        virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

        virtual Ptr<Node> GetNode (void) const;
        virtual void SetNode (Ptr<Node> node);

        virtual bool NeedsArp (void) const;

        virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

        virtual Address GetMulticast (Ipv6Address addr) const;

        virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
        virtual bool SupportsSendFrom (void) const;

        protected:
        void DoMpiReceive (Ptr<Packet> p);
        

private:

  MyNetDevice& operator = (const MyNetDevice &o);

  MyNetDevice (const MyNetDevice &o);

  virtual void DoDispose (void);
        void AddHeader (Ptr<Packet> p, uint16_t protocolNumber);
        bool ProcessHeader (Ptr<Packet> p, uint16_t& param);
        Address GetRemote (void) const;
        bool TransmitStart (Ptr<Packet> p);
        void TransmitComplete (void);
        void NotifyLinkUp (void);
        // void SetQueue(Ptr<DDLQueue> q);
        enum TxMachineState
        {
            READY,   /**< The transmitter is ready to begin transmission of a packet */
            BUSY     /**< The transmitter is busy transmitting a packet */
        };
  /**
   * The state of the Net Device transmit state machine.
   */
  TxMachineState m_txMachineState;

  /**
   * The data rate that the Net Device uses to simulate packet transmission
   * timing.
   */
  DataRate       m_bps;

  /**
   * The interframe gap that the Net Device uses to throttle packet
   * transmission
   */
  Time           m_tInterframeGap;

  /**
   * The PointToPointChannel to which this PointToPointNetDevice has been
   * attached.
   */
  Ptr<MyChannel> m_channel;

  /**
   * The Queue which this PointToPointNetDevice uses as a packet source.
   * Management of this Queue has been delegated to the PointToPointNetDevice
   * and it has the responsibility for deletion.
   * \see class DropTailQueue
   */
  Ptr<DDLQueue> m_queue;

  /**
   * Error model for receive packet events
   */
  Ptr<ErrorModel> m_receiveErrorModel;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * at the L3/L2 transition are dropped before being queued for transmission.
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a promiscuous trace (which doesn't mean a lot here
   * in the point-to-point device).
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a non-promiscuous trace (which doesn't mean a lot 
   * here in the point-to-point device).
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but are dropped before being forwarded up to higher layers (at the L2/L3 
   * transition).
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * The trace source fired when a packet begins the transmission process on
   * the medium.
   */
  TracedCallback<Ptr<const Packet> > m_phyTxBeginTrace;

  /**
   * The trace source fired when a packet ends the transmission process on
   * the medium.
   */
  TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet before it tries
   * to transmit it.
   */
  TracedCallback<Ptr<const Packet> > m_phyTxDropTrace;

  /**
   * The trace source fired when a packet begins the reception process from
   * the medium -- when the simulated first bit(s) arrive.
   */
  TracedCallback<Ptr<const Packet> > m_phyRxBeginTrace;

  /**
   * The trace source fired when a packet ends the reception process from
   * the medium.
   */
  TracedCallback<Ptr<const Packet> > m_phyRxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet it has received.
   * This happens if the receiver is not enabled or the error model is active
   * and indicates that the packet is corrupt.
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected 
   * to the device.  Unlike your average everyday sniffer, this trace source 
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device \c hard_start_xmit where 
   * \c dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in \c netif_receive_skb.
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device \c hard_start_xmit where 
   * \c dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in \c netif_receive_skb.
   */
  uint16_t PppToEther (uint16_t proto);
  uint16_t EtherToPpp (uint16_t proto);

  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  Ptr<Node> m_node;         //!< Node owning this NetDevice
  Mac48Address m_address;   //!< Mac48Address of this NetDevice
  NetDevice::ReceiveCallback m_rxCallback;   //!< Receive callback
  NetDevice::PromiscReceiveCallback m_promiscCallback;  //!< Receive callback
                                                        //   (promisc data)
  uint32_t m_ifIndex; //!< Index of the interface
  bool m_linkUp;      //!< Identify if the link is up or not
  TracedCallback<> m_linkChangeCallbacks;  //!< Callback for the link change event

  static const uint16_t DEFAULT_MTU = 1500; //!< Default MTU

  /**
   * \brief The Maximum Transmission Unit
   *
   * This corresponds to the maximum 
   * number of bytes that can be transmitted as seen from higher layers.
   * This corresponds to the 1500 byte MTU size often seen on IP over 
   * Ethernet.
   */
            uint32_t m_mtu;

            Ptr<Packet> m_currentPkt; //!< Current packet processed
    };
}





#endif
