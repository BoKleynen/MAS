/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ant-packet.h"
#include "ns3/address-utils.h"

namespace ns3 {
namespace ant_routing {

AntHeader::AntHeader (std::vector<Ipv4Address> visitedNodes, AntType antType,
          uint8_t hopCount, uint8_t broadcastCount,
          uint8_t backwardCount, uint32_t generation,
          Ipv4Address dst, Ipv4Address origin,
          Time timeEstimate)
  : m_antType (antType),
    m_hopCount (hopCount),
    m_broadcastCount (broadcastCount),
    m_backwardCount (backwardCount),
    m_generation (generation),
    m_origin (origin),
    m_dst (dst),
    m_timeEstimate (timeEstimate),
    m_visitedNodes (visitedNodes)
    { }

AntHeader::AntHeader()
  : m_antType (AntType::ReactiveForwardAnt),
    m_hopCount (0),
    m_broadcastCount (0),
    m_backwardCount (0),
    m_generation (0),
    m_origin (Ipv4Address()),
    m_dst (Ipv4Address()),
    m_timeEstimate (Time()),
    m_visitedNodes (std::vector<Ipv4Address> ()) {}



TypeId
AntHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ant_routing::AntHeader")
    .SetParent<Header> ()
    .SetGroupName ("AntRouting") // TODO: what is the group name?
    .AddConstructor<AntHeader> ();
  return tid;
}

TypeId
AntHeader::GetInstanceTypeId () const
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
  auto nanoTime = m_timeEstimate.GetNanoSeconds();
  i.Write((const uint8_t *) &(nanoTime), sizeof(uint64_t));
  WriteTo(i, m_origin);
  WriteTo(i, m_dst);
  for (auto addr : m_visitedNodes) {
    WriteTo (i, addr);
  }
}

uint32_t
AntHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_antType = static_cast<AntType>(i.ReadU8 ());
  m_hopCount = i.ReadU8 ();
  m_broadcastCount = i.ReadU8 ();
  m_backwardCount = i.ReadU8 ();
  m_generation = i.ReadNtohU32 ();
  int64_t rcvdTime;
  i.Read ((uint8_t *)&rcvdTime, 8);
  m_timeEstimate = NanoSeconds (rcvdTime);
  ReadFrom (i, m_origin);
  ReadFrom (i, m_dst);
  Ipv4Address addr;
  for (int index=1; index < m_hopCount; index++) { // (m_hopCount - 1) visited nodes
    ReadFrom(i, addr);
    m_visitedNodes.push_back(addr);
  }

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
AntHeader::Print (std::ostream &os) const
{
  os << "we don't do printing";
}

AntType AntHeader::GetAntType() {
  return m_antType;
}
uint8_t AntHeader::GetHopCount() {
  return m_hopCount;
}
uint8_t AntHeader::GetBroadcastCount() {
  return m_broadcastCount;
}
uint8_t AntHeader::GetBackwardCount() {
  return m_backwardCount;
}
uint32_t AntHeader::GetGeneration() {
  return m_generation;
}
Ipv4Address AntHeader::GetOrigin() {
  return m_origin;
}
Ipv4Address AntHeader::GetDestination() {
  return m_dst;
}
Time AntHeader::GetTimeEstimate() {
  return m_timeEstimate;
}
std::vector<Ipv4Address> AntHeader::GetVisitedNodes() {
  return m_visitedNodes;
}

void AntHeader::SetAntType(AntType antType) {
  m_antType = antType;
}
void AntHeader::SetHopCount(uint8_t hopCount) {
  m_hopCount = hopCount;
}
void AntHeader::SetBroadcastCount(uint8_t broadcastCount) {
  m_broadcastCount = broadcastCount;
}
void AntHeader::SetBackwardCount(uint8_t backwardCount) {
  m_backwardCount = backwardCount;
}
void AntHeader::SetGeneration(uint32_t generation) {
  m_generation = generation;
}
void AntHeader::SetOrigin(Ipv4Address origin) {
  m_origin = origin;
}
void AntHeader::SetDestination(Ipv4Address dest) {
  m_dst = dest;
}
void AntHeader::SetTimeEstimate(Time timeEstimate) {
  m_timeEstimate = timeEstimate;
}
void AntHeader::SetVisitedNodes(const std::vector<Ipv4Address>& visited) {
  m_visitedNodes = visited;
}
void AntHeader::SetVisitedNodes(std::vector<Ipv4Address>&& visited) {
  m_visitedNodes = std::move(visited);
}

void AntHeader::AddVisitedNode(Ipv4Address addr) {
  m_visitedNodes.push_back(addr);
}

} // namespace ant_routing
} // namespace
