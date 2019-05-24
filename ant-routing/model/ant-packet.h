/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANTPACKET_H
#define ANTPACKET_H

#include <vector>
#include <iostream>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

namespace ns3 {
namespace ant_routing {

class enum AntType
{
  ReactiveForwardAnt = 0,
  ProactiveForwardAnt = 1,
  ProactiveBroadcastAnt = 2,
  BackwardAnt = 3,
  RouteRepairAnt = 4,
  HelloAnt = 5,
};

/**
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Ant Type   |   Hop count   |Broadcast Count|Backward  Count|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Generation                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Time Estimate                         |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Originator Address                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Destination Address                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
:                       Visited Nodes                           :
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
class AntHeader : public Header
{
public:
  // full fledged constructor
  AntHeader (std::vector<Ipv4Address> visitedNodes, AntType antType,
            uint8_t hopCount, uint8_t broadcastCount,
            uint8_t m_backwardCount, uint32_t generation,
            Ipv4Address dst, Ipv4Address origin,
            Time timeEstimate);

  AntHeader(); // default constructor

  AntHeader(const AntHeader& ah) = default;
  AntHeader(AntHeader&& ah) = default;

  AntHeader& operator=(const AntHeader& ah) = default;
  AntHeader& operator=(AntHeader&& ah) = default;


  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  AntType GetAntType() const;
  uint8_t GetHopCount() const;
  uint8_t GetBroadcastCount() const;
  uint8_t GetBackwardCount() const;
  uint32_t GetGeneration() const;
  Ipv4Address GetOrigin() const;
  Ipv4Address GetDestination() const;
  Time GetTimeEstimate() const;
  std::vector<Ipv4Address> GetVisitedNodes();

  void SetAntType(AntType antType);
  void SetHopCount(uint8_t hopCount);
  void SetBroadcastCount(uint8_t broadcastCount);
  void SetBackwardCount(uint8_t backwardCount);
  void SetGeneration(uint32_t generation);
  void SetOrigin(Ipv4Address origin);
  void SetDestination(Ipv4Address dest);
  void SetTimeEstimate(Time timeEstimate);
  void SetVisitedNodes(const std::vector<Ipv4Address>& visited);
  void SetVisitedNodes(std::vector<Ipv4Address>&& visited);

  void AddVisitedNode(Ipv4Address addr);

private:
  AntType                   m_antType;
  uint8_t                   m_hopCount;
  uint8_t                   m_broadcastCount;
  uint8_t                   m_backwardCount;
  uint32_t                  m_generation;
  Ipv4Address               m_origin;
  Ipv4Address               m_dst;
  Time                      m_timeEstimate;
  std::vector<Ipv4Address>  m_visitedNodes;
}; // class AntHeader

class HelloHeader {
public:
  HelloHeader ();

  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
private:

}; // class HelloHeader

class LinkFailureNotification : public Header {
public:
  struct Message
  {
    Ipv4Address dest;
    Time        bestTimeEstimate;
    uint8_t     bestHopEstimate;

    static uint32_t GetSerializedSize (void);
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
  };

  LinkFailureNotification ();
  LinkFailureNotification (Ipv4Address origin);
  LinkFailureNotification (Ipv4Address origin, std::vector<Message> messages);

  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  bool operator ==(const LinkFailureNotification& rhs)
  {
    return m_origin == rhs.m_origin && m_messages == rhs.m_messages;
  }
private:
  Ipv4Address m_origin;
  std::vector<Message> m_messages;

}; // LinkFailureNotification

std::ostream& operator <<(std::ostream& os, const LinkFailureNotification::Message& message);


inline bool operator==(const LinkFailureNotification::Message& lhs, const LinkFailureNotification::Message& rhs) {
  return lhs.dest == rhs.dest
    && lhs.bestTimeEstimate == rhs.bestTimeEstimate
    && lhs.bestHopEstimate == rhs.bestHopEstimate;
}

class HelloAnt : public Header
{
public:
  HelloAnt (Ipv4Address origin);
  HelloAnt ();

  HelloAnt(const HelloAnt& ah) = default;
  HelloAnt(HelloAnt&& ah) = default;

  HelloAnt& operator=(const HelloAnt& ah) = default;
  HelloAnt& operator=(HelloAnt&& ah) = default;

  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  AntType GetAntType ();
  Ipv4Address GetOrigin ();

  void SetOrigin (Ipv4Address origin);
private:
  AntType     m_antType;
  Ipv4Address m_origin;
};

} // namespace ant_routing
} // namespace
#endif /* ANTPACKET_H */
