#include "int-tcp-option.h"

namespace ns3{

IntTcpOption::IntTcpOption() {
    m_kind = TcpOption::INT;
}

IntTcpOption::~IntTcpOption ()
{
}

TypeId
IntTcpOption::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IntTcpOption")
    .SetParent<TcpOption> ()
    .SetGroupName ("Internet")
    .AddConstructor<IntTcpOption> ()
  ;
  return tid;
}

TypeId
IntTcpOption::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t
IntTcpOption::GetKind(void) const
{
  return TcpOption::INT;
}

void
IntTcpOption::Print (std::ostream &os) const
{
  os << "Int TCP option";
}

void
IntTcpOption::SetMode(uint8_t mode) {
    
}

void
IntTcpOption::SetDeadline(uint32_t deadline) {
  m_deadline = deadline;
}

uint32_t IntTcpOption::GetDeadline() const {
  return m_deadline;
}

uint32_t
IntTcpOption::GetSerializedSize (void) const
{
  return sizeof(m_kind) + sizeof(m_deadline);
}

void
IntTcpOption::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (GetKind ());
  i.WriteU32(m_deadline);
}

uint32_t
IntTcpOption::Deserialize(Buffer::Iterator i)
{
    m_kind = i.ReadU8();
    m_deadline = i.ReadU32();
    
    return sizeof(m_kind) + sizeof(m_deadline);
}
} 