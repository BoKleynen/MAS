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

enum class AntType : uint8_t
{
  ReactiveForwardAnt = 0,
  ProactiveForwardAnt = 1,
  ProactiveBroadcastAnt = 2,
  BackwardAnt = 3,
  RouteRepairAnt = 4,
  HelloAnt = 5,
  LinkFailureAnt = 6,
};

// AntTypeHeader ----------------------------------------------------------------

/*
 * specifies the type of header that follows and the ip adress of the sender.
 */
class AntTypeHeader : public Header
{
public:
  AntTypeHeader ();
  AntTypeHeader (AntType antType);

  AntTypeHeader(const AntTypeHeader& ah) = default;
  AntTypeHeader(AntTypeHeader&& ah) = default;

  AntTypeHeader& operator=(const AntTypeHeader& ah) = default;
  AntTypeHeader& operator=(AntTypeHeader&& ah) = default;

  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator i);
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  void Print (std::ostream &os) const;

  AntType GetAntType () const;
  void SetAntType (AntType antType);

  inline bool operator== (const AntTypeHeader& rhs)
  {
    return m_antType == rhs.m_antType;
  }

private:
  AntType       m_antType;
};

/**
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Hop count   |Broadcast Count|Backward  Count|               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Generation                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Time Estimate                         |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
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
  AntHeader (std::vector<Ipv4Address> visitedNodes,
            uint8_t hopCount, uint8_t broadcastCount,
            uint8_t visitedSize, uint32_t generation,
            Ipv4Address source,Ipv4Address dst, Time timeEstimate);

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

  uint8_t GetHopCount() const;
  uint8_t GetBroadcastCount() const;
  uint8_t GetVisitedSize() const;
  uint32_t GetGeneration() const;
  Ipv4Address GetSource() const;
  Ipv4Address GetDestination() const;
  Time GetTimeEstimate() const;
  std::vector<Ipv4Address> GetVisitedNodes();

  void SetHopCount(uint8_t hopCount);
  void SetBroadcastCount(uint8_t broadcastCount);
  // void SetVisitedSize(uint8_t backwardCount);
  void SetGeneration(uint32_t generation);
  void SetSource(Ipv4Address source);
  void SetDestination(Ipv4Address dest);
  void SetTimeEstimate(Time timeEstimate);
  void SetVisitedNodes(const std::vector<Ipv4Address>& visited);
  void SetVisitedNodes(std::vector<Ipv4Address>&& visited);

  void AddVisitedNode(Ipv4Address addr);
  void PopVisitedNode();

  uint8_t                   m_hopCount;
  uint8_t                   m_broadcastCount;
  uint8_t                   m_visitedSize;
  uint32_t                  m_generation;
  Ipv4Address               m_source;
  Ipv4Address               m_dst;
  Time                      m_timeEstimate;
  std::vector<Ipv4Address>  m_visitedNodes;
}; // class AntHeader

// Hello declaration -----------------------------------------------------------
class HelloHeader : public Header {
public:

  HelloHeader();
  HelloHeader(Ipv4Address source);

  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  Ipv4Address GetSource() const;
  void SetSource(Ipv4Address source);

private:
  Ipv4Address m_source;
};

//LinkFaiure declaration -------------------------------------------------------
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
  LinkFailureNotification (Ipv4Address source, std::vector<Message> messages);

  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  Ipv4Address GetSource() const;
  std::vector<Message> GetMessages();

  void SetSource(Ipv4Address source);
  void SetMessages(std::vector<Message> messages);


  inline bool operator ==(const LinkFailureNotification& rhs)
  {
    return m_messages == rhs.m_messages;
  }

private:
  Ipv4Address m_source;
  std::vector<Message> m_messages;
}; // LinkFailureNotification

std::ostream& operator <<(std::ostream& os, const LinkFailureNotification::Message& message);


inline bool operator==(const LinkFailureNotification::Message& lhs, const LinkFailureNotification::Message& rhs) {
  return lhs.dest == rhs.dest
    && lhs.bestTimeEstimate == rhs.bestTimeEstimate
    && lhs.bestHopEstimate == rhs.bestHopEstimate;
}

} // namespace ant_routing
} // namespace
#endif /* ANTPACKET_H */
