/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ant-packet.h"
#include "ns3/address-utils.h"

#define IPV4_ADDRESS_SIZE 4

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
    + (1 + m_hopCount) * IPV4_ADDRESS_SIZE;  // (2 + (m_hopCount - 1))
}

void
AntHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (static_cast<uint8_t>(m_antType));
  i.WriteU8 (m_hopCount);
  i.WriteU8 (m_broadcastCount);
  i.WriteU8 (m_backwardCount);
  i.WriteHtonU32 (m_generation);
  // https://www.nsnam.org/doxygen/lte-rlc-tag_8cc_source.html#l00066
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
  auto i = start;
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

AntType AntHeader::GetAntType() const {
  return m_antType;
}
uint8_t AntHeader::GetHopCount() const {
  return m_hopCount;
}
uint8_t AntHeader::GetBroadcastCount() const {
  return m_broadcastCount;
}
uint8_t AntHeader::GetBackwardCount() const {
  return m_backwardCount;
}
uint32_t AntHeader::GetGeneration() const {
  return m_generation;
}
Ipv4Address AntHeader::GetOrigin() const {
  return m_origin;
}
Ipv4Address AntHeader::GetDestination() const {
  return m_dst;
}
Time AntHeader::GetTimeEstimate() const {
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

// --------------- LinkFailureNotification ---------------

LinkFailureNotification::LinkFailureNotification ()
  : m_origin (Ipv4Address ()),
    m_messages (std::vector<Message> ()) {}

LinkFailureNotification::LinkFailureNotification (Ipv4Address origin)
  : m_origin (origin),
    m_messages (std::vector<Message> ()) {}

LinkFailureNotification::LinkFailureNotification (Ipv4Address origin, std::vector<Message> messages)
  : m_origin (origin),
    m_messages (messages) {}

TypeId
LinkFailureNotification::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ant_routing::LinkFailureNotification")
    .SetParent<Header> ()
    .SetGroupName ("AntRouting")
    .AddConstructor<LinkFailureNotification> ();
  return tid;
}

TypeId
LinkFailureNotification::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
LinkFailureNotification::GetSerializedSize () const
{
  return IPV4_ADDRESS_SIZE + m_messages.size() * 13;
}

void
LinkFailureNotification::Serialize (Buffer::Iterator i) const
{
  WriteTo(i, m_origin);
  i.WriteU8 (m_messages.size());
  for (auto iter = m_messages.begin (); iter != m_messages.end (); iter++)
  {
    iter->Serialize (i);
  }
}

uint32_t
LinkFailureNotification::Deserialize (Buffer::Iterator start)
{
  auto i = start;
  ReadFrom(i, m_origin);
  auto n_messages = i.ReadU8 ();
  for (int n = 1; n < n_messages; n++) {
    auto message = Message ();
    message.Deserialize (i);
    m_messages.push_back(message);
  }

  uint32_t dist = i.GetDistanceFrom (start);
  return dist;
}

void
LinkFailureNotification::Print (std::ostream &os) const
{
  os << "bla bla bla";
}

// --------------- LinkFailureNotification::Message ---------------


uint32_t
LinkFailureNotification::Message::GetSerializedSize ()
{
  return IPV4_ADDRESS_SIZE + sizeof(uint64_t) + sizeof(uint8_t);
}

void
LinkFailureNotification::Message::Serialize (Buffer::Iterator i) const
{
  WriteTo (i, dest);
  auto nanoTime = bestTimeEstimate.GetNanoSeconds ();
  i.Write ((const uint8_t *) &(nanoTime), sizeof(uint64_t));
  i.WriteU8 (bestHopEstimate);
}

uint32_t
LinkFailureNotification::Message::Deserialize (Buffer::Iterator start)
{
  auto i = start;
  ReadFrom (i, dest);
  int64_t rcvdTime;
  i.Read ((uint8_t *)&rcvdTime, 8);
  bestTimeEstimate = NanoSeconds (rcvdTime);
  bestHopEstimate = i.ReadU8 ();

  uint32_t dist = i.GetDistanceFrom (start);
  return dist;
}


std::ostream& operator <<(std::ostream& os, const LinkFailureNotification::Message& message) {
return os << "Message { "
          << message.dest
          << message.bestTimeEstimate
          << message.bestHopEstimate
          << " }";
}


// --------------- HelloAnt ---------------

HelloAnt::HelloAnt ()
  : m_antType (AntType::HelloAnt),
    m_origin (Ipv4Address ())
{
}

HelloAnt::HelloAnt (Ipv4Address origin)
  : m_antType (AntType::HelloAnt),
    m_origin (origin)
{
}

TypeId
HelloAnt::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ant_routing::AntHeader")
    .SetParent<Header> ()
    .SetGroupName ("AntRouting")
    .AddConstructor<HelloAnt> ();
  return tid;
}

TypeId
HelloAnt::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
HelloAnt::GetSerializedSize () const
{
  return sizeof(AntType) + IPV4_ADDRESS_SIZE;
}

void
HelloAnt::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (static_cast<uint8_t>(m_antType));
  WriteTo (i, m_origin);
}

uint32_t
HelloAnt::Deserialize (Buffer::Iterator start)
{
  auto i = start;
  m_antType = static_cast<AntType>(i.ReadU8 ());
  ReadFrom (i, m_origin);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloAnt::Print (std::ostream &os) const
{
  os << "HelloAnt from somewhere";
}

AntType
HelloAnt::GetAntType ()
{
  return m_antType;
}

Ipv4Address
HelloAnt::GetOrigin ()
{
  return m_origin;
}

void
HelloAnt::SetOrigin (Ipv4Address origin)
{
  m_origin = origin;
}

} // namespace ant_routing
} // namespace
