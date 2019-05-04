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
   static TypeId GetTypeId ();
   uint32_t GetSerializedSize () const;
   void Serialize (Buffer::Iterator start) const;
   uint32_t Deserialize (Buffer::Iterator start);
   void Print (std::ostream &os) const;
private:
  AntType                   antType;
  uint8_t                   hopCount;
  uint8_t                   broadcastCount;
  uint8_t                   backwardCount;
  uint32_t                  generation;
  Ipv4Address               origin;
  Ipv4Address               dst;
  Time                      timeEstimate;
  std::vector<Ipv4Address>  visitedNodes;
} // class ReactiveForwardAnt
} // namespace ant_routing
} // namespace
#endif /* ANTPACKET_H */
