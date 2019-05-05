/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ant-packet.h"

namespace ns3 {
namespace ant_routing {

AntHeader (AntType antType = AntType.ReactiveForwardAnt, uint8_t hopCount = 0,
          uint8_t broadcastCount = 0, uint8_t backwardCount = 0,
          uint32_t generation = 0, Ipv4Address dst = Ipv4Address (),
          Ipv4Address m_origin = Ipv4Address (), Time timeEstimate = Time ())
  : m_visitedNodes (std::vector<Ipv4Address> ()),
    m_antType (antType),
    m_hopCount (hopCount),
    m_broadcastCount (broadcastCount),
    m_backwardCount (backwardCount),
    m_generation (generation),
    m_dst (dst),
    m_origin (origin),
    m_timeEstimate (timeEstimate)
{
}

AntHeader (std::vector<Ipv4Address> visitedNodes, AntType antType = AntType.ReactiveForwardAnt,
          uint8_t hopCount = 0, uint8_t broadcastCount = 0,
          uint8_t m_backwardCount = 0, uint32_t generation = 0,
          Ipv4Address dst = Ipv4Address (), Ipv4Address origin = Ipv4Address (),
          Time timeEstimate = Time ())
  : m_visitedNodes (visitedNodes),
    m_antType (antType),
    m_hopCount (hopCount),
    m_broadcastCount (broadcastCount),
    m_backwardCount (backwardCount),
    m_generation (generation),
    m_dst (dst),
    m_origin (origin),
    m_timeEstimate (timeEstimate)
{
}




TypeId
AntHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ant_routing::AntHeader")
    .SetParent<Header> ()
    .SetGroupName ("ant_routing") // TODO: what is the group name?
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
  return sizeof(AntType)
    + 3*sizeof(uint8_t)
    + sizeof(uint32_t)
    + sizeof(Time)
    + (1 + m_hopCount) * sizeof(Ipv4Address);  // (2 + (m_hopCount - 1))
}

void
AntHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_antType);
  i.WriteU8 (m_hopCount);
  i.WriteU8 (m_broadcastCount);
  i.WriteU8 (m_backwardCount);
  i.WriteHtonU32 (m_generation);
  WriteTo(i,m_timeEstimate);
  WriteTo(i, m_origin);
  WriteTo(i, m_dst);
  for (auto addr : m_visitedNodes) {
    WriteTo (i, addr)
  }
}

uint32_t
AntHeader::Deserialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  m_antType = i.ReadU8 ();
  m_hopCount = i.ReadU8 ();
  m_broadcastCount = i.Readu8 ();
  m_backwardCount = i.ReadU8 ();
  m_generation = i.ReadNtohU32 ();
  ReadFrom (i, timeEstimate);
  ReadFrom (i, m_origin);
  ReadFrom (i, dst);
  Ipv4Address addr;
  for (int i=1; i < m_hopCount; i++) { // (m_hopCount - 1) visited nodes
    ReadFrom(i, addr)
    visitedNodes.push_back(addr)
  }

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
AntHeader::Print (std::ostream &os) const
{
  os << "we don't do printing"
}

} // namespace ant_routing
} // namespace
