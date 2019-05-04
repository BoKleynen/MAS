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

enum AntType
{
  ReactiveForwardAnt = 0,
  ProactiveForwardAnt = 1,
  ProactiveBroadcastAnt = 2,
  BackwardAnt = 3,
  RouteRepairAnt = 4;
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
  AntHeader (AntType antType = AntType.ReactiveForwardAnt, uint8_t hopCount = 0,
            uint8_t broadcastCount = 0, uint8_t backwardCount = 0,
            uint32_t m_generation = 0, Ipv4Address dst = Ipv4Address (),
            Ipv4Address m_origin = Ipv4Address (), Time timeEstimate = Time ());

  AntHeader (AntType m_antType = AntType.ReactiveForwardAnt,uint8_t m_hopCount = 0,
            uint8_t m_broadcastCount = 0, uint8_t m_backwardCount = 0,
            uint32_t m_generation = 0, Ipv4Address dst = Ipv4Address (),
            Ipv4Address m_origin = Ipv4Address (), Time timeEstimate = Time (),
            std::vector<Ipv4Address> visitedNodes);

  static TypeId GetTypeId ();
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
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
} // class ReactiveForwardAnt
} // namespace ant_routing
} // namespace
#endif /* ANTPACKET_H */
