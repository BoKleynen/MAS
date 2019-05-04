/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ant-packet.h"
#include "ns3/packet.h"

namespace ns3 {
namespace ant_routing {

TypeId
AntHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ant_routing::AntHeader")
    .SetParent<Header> ()
    .SetGroupName ("ant_routing") // TODO: what is the group namen?
    .AddConstructor<AntHeader> ();
  return tid;
}

TypeId
Antheader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
AntHeader::GetSerializedSize () const
{
  return 0;
}

void
AntHeader::Serialize (Buffer::Iterator i) const
{

}

uint32_t
AntHeader::Deserialize (Buffer::Iterator start) const
{

}

void
AntHeader::Print (std::ostream &os) const
{
  os << ""
}

} // namespace ant_routing
} // namespace
