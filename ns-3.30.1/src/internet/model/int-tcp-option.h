#ifndef INT_TCP_OPTION
#define INT_TCP_OPTION


#include <stdint.h>
#include "ns3/tcp-option.h"

#include "ns3/int-header.h"



namespace ns3{

class IntTcpOption: public TcpOption{
public:
  IntTcpOption();
  virtual ~IntTcpOption();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  virtual uint8_t GetKind (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  void SetMode(uint8_t mode);
  void SetDeadline(uint32_t deadline);
  uint32_t GetDeadline() const;

private:
  uint8_t m_kind; // Kind number 15
  uint32_t m_deadline;
};

}
#endif