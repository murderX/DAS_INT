#ifndef MY_CHANNEL_H
#define MY_CHANNEL_H

#include <list>
#include "ns3/channel.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class MyNetDevice;
class Packet;

/**
 * \ingroup point-to-point
 * \brief Simple Point To Point Channel.
 *
 * This class represents a very simple point to point channel.  Think full
 * duplex RS-232 or RS-422 with null modem and no handshaking.  There is no
 * multi-drop capability on this channel -- there can be a maximum of two 
 * point-to-point net devices connected.
 *
 * There are two "wires" in the channel.  The first device connected gets the
 * [0] wire to transmit on.  The second device gets the [1] wire.  There is a
 * state (IDLE, TRANSMITTING) associated with each wire.
 *
 * \see Attach
 * \see TransmitStart
 */
class MyChannel : public Channel 
{
public:
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Create a PointToPointChannel
   *
   * By default, you get a channel that has an "infinitely" fast 
   * transmission speed and zero delay.
   */
  MyChannel ();

  /**
   * \brief Attach a given netdevice to this channel
   * \param device pointer to the netdevice to attach to the channel
   */
  void Attach (Ptr<MyNetDevice> device);

  /**
   * \brief Transmit a packet over this channel
   * \param p Packet to transmit
   * \param src Source PointToPointNetDevice
   * \param txTime Transmit time to apply
   * \returns true if successful (currently always true)
   */
  virtual bool TransmitStart (Ptr<const Packet> p, Ptr<MyNetDevice> src, Time txTime);

  /**
   * \brief Get number of devices on this channel
   * \returns number of devices on this channel
   */
  virtual std::size_t GetNDevices (void) const;

  /**
   * \brief Get PointToPointNetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to PointToPointNetDevice requested
   */
  Ptr<MyNetDevice> GetPointToPointDevice (std::size_t i) const;

  /**
   * \brief Get NetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to NetDevice requested
   */
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

protected:
  /**
   * \brief Get the delay associated with this channel
   * \returns Time delay
   */
  Time GetDelay (void) const;

  /**
   * \brief Check to make sure the link is initialized
   * \returns true if initialized, asserts otherwise
   */
  bool IsInitialized (void) const;

  /**
   * \brief Get the net-device source 
   * \param i the link requested
   * \returns Ptr to PointToPointNetDevice source for the 
   * specified link
   */
  Ptr<MyNetDevice> GetSource (uint32_t i) const;

  /**
   * \brief Get the net-device destination
   * \param i the link requested
   * \returns Ptr to PointToPointNetDevice destination for 
   * the specified link
   */
  Ptr<MyNetDevice> GetDestination (uint32_t i) const;

  /**
   * TracedCallback signature for packet transmission animation events.
   *
   * \param [in] packet The packet being transmitted.
   * \param [in] txDevice the TransmitTing NetDevice.
   * \param [in] rxDevice the Receiving NetDevice.
   * \param [in] duration The amount of time to transmit the packet.
   * \param [in] lastBitTime Last bit receive time (relative to now)
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  typedef void (* TxRxAnimationCallback)
    (Ptr<const Packet> packet,
     Ptr<NetDevice> txDevice, Ptr<NetDevice> rxDevice,
     Time duration, Time lastBitTime);
                    
private:
  /** Each point to point link has exactly two net devices. */
  static const std::size_t N_DEVICES = 2;

  Time          m_delay;    //!< Propagation delay
  std::size_t        m_nDevices; //!< Devices of this channel

  /**
   * The trace source for the packet transmission animation events that the 
   * device can fire.
   * Arguments to the callback are the packet, transmitting
   * net device, receiving net device, transmission time and 
   * packet receipt time.
   *
   * \see class CallBackTraceSource
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  TracedCallback<Ptr<const Packet>,     // Packet being transmitted
                 Ptr<NetDevice>,  // Transmitting NetDevice
                 Ptr<NetDevice>,  // Receiving NetDevice
                 Time,                  // Amount of time to transmit the pkt
                 Time                   // Last bit receive time (relative to now)
                 > m_txrxPointToPoint;

  /** \brief Wire states
   *
   */
  enum WireState
  {
    /** Initializing state */
    INITIALIZING,
    /** Idle state (no transmission from NetDevice) */
    IDLE,
    /** Transmitting state (data being transmitted from NetDevice. */
    TRANSMITTING,
    /** Propagating state (data is being propagated in the channel. */
    PROPAGATING
  };

  /**
   * \brief Wire model for the PointToPointChannel
   */
  class Link
  {
public:
    /** \brief Create the link, it will be in INITIALIZING state
     *
     */
    Link() : m_state (INITIALIZING), m_src (0), m_dst (0) {}

    WireState                  m_state; //!< State of the link
    Ptr<MyNetDevice> m_src;   //!< First NetDevice
    Ptr<MyNetDevice> m_dst;   //!< Second NetDevice
  };

  Link    m_link[N_DEVICES]; //!< Link model
};

} // namespace ns3

#endif /* POINT_TO_POINT_CHANNEL_H */