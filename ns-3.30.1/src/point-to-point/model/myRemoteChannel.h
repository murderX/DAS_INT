#ifndef MY_REMOTE_CHANNEL_H
#define MY_REMOTE_CHANNEL_H

#include "myChannel.h"

namespace ns3 {

/**
 * \ingroup point-to-point
 *
 * \brief A Remote Point-To-Point Channel
 * 
 * This object connects two point-to-point net devices where at least one
 * is not local to this simulator object. It simply override the transmit
 * method and uses an MPI Send operation instead.
 */
class MyRemoteChannel : public MyChannel
{
public:
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /** 
   * \brief Constructor
   */
  MyRemoteChannel ();

  /** 
   * \brief Deconstructor
   */
  ~MyRemoteChannel ();

  /**
   * \brief Transmit the packet
   *
   * \param p Packet to transmit
   * \param src Source PointToPointNetDevice
   * \param txTime Transmit time to apply
   * \returns true if successful (currently always true)
   */
  virtual bool TransmitStart (Ptr<const Packet> p, Ptr<MyNetDevice> src,
                              Time txTime);
};

} // namespace ns3

#endif