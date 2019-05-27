/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "backward-ant.h"
#include "ant-routing.h"
#include "ant-netdevice.h"
#include "ant-routing-table.h"
#include "reactive-queue.h"

namespace ns3 {
namespace ant_routing {

BackwardAnt::BackwardAnt(const AntHeader& header)
  : m_header(header) {
  NS_LOG_UNCOND("Backward ant created! -----------------------------");
}


BackwardAnt::BackwardAnt(Ptr<Packet> packet)
  : m_header(AntHeader()){
  NS_LOG_UNCOND("RECEIVED NEXT HOP PACKET---------------------------");
  packet -> RemoveHeader(m_header);
}

/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void
BackwardAnt::Visit(AnthocnetRouting router) {
  NS_LOG_UNCOND("Visiting router ----------------------------------");
  if(router.GetAddress() == GetHeader().GetDestination()){
    HandleAtDestination(router);
  }else{
    UpdateDistanceMetrics(router);
    UpdateRoutingTable(router);
    ReleasePending(router);
    NextHop(router);
  }
  // first get the header up to date for updating the routing table entries

}

AntHeader
BackwardAnt::GetHeader() {
  return m_header;
}

void
BackwardAnt::HandleAtDestination(AnthocnetRouting router) {
  auto nxt = GetHeader().GetVisitedNodes().back();
  auto optNeighbor = router.GetRoutingTable().GetNeighbor(nxt);

  if(!optNeighbor.IsValid()) {
    NS_LOG_UNCOND("Neighbor not available for backward ant, dropped packet");
    return;
  }

  auto neighbor = router.GetRoutingTable();
  auto packet = NextHopPacket(router);
  UnicastPacket(packet, router, optNeighbor.Get());

}

void
BackwardAnt::UpdateDistanceMetrics(AnthocnetRouting router) {
  auto dev = router.GetDevice();
  m_header.m_hopCount ++;
  m_header.m_timeEstimate += (dev.QueueSize() + 1) * dev.SendingTimeEst();
}

void
BackwardAnt::UpdateRoutingTable(AnthocnetRouting router) {



  if(GetHeader().GetVisitedNodes().empty()) {
    NS_ASSERT_MSG(false, "Backward ant should never have empty visited nodes during traversal"); // should not happen
    return;
  }
  auto rTable = router.GetRoutingTable();
  rTable.UpdatePheromoneEntry(GetHeader().GetVisitedNodes().back(),
                              GetHeader().GetDestination(),
                              GetHeader().GetTimeEstimate(),
                              GetHeader().GetHopCount());
}

void
BackwardAnt::ReleasePending(AnthocnetRouting router) {
  auto rQueue = router.GetReactiveQueue();
  rQueue.EntryAddedFor(GetHeader().GetDestination(), router);
}

void
BackwardAnt::NextHop(AnthocnetRouting router) {
  NS_ASSERT_MSG(GetHeader().GetVisitedNodes().size() > 0, "There should always be an entry to pop in the backward ant");
  if(router.GetAddress() != GetHeader().GetDestination()){
    GetHeader().m_visitedNodes.pop_back();
    m_header.m_visitedSize--;
  }
  if(GetHeader().GetVisitedNodes().empty()){
    return; // nothing left to do, reached the end
  }

  auto nxt = GetHeader().m_visitedNodes.back();

  auto rTable = router.GetRoutingTable();
  auto optNeighbor = rTable.GetNeighbor(nxt);

  if(!optNeighbor.IsValid()) {
    NS_LOG_UNCOND("Neighbor got offline, dropping packet");
    return; // neighbor got offline in the meantime
  }

  auto packet = NextHopPacket(router);
  UnicastPacket(packet, router, optNeighbor.Get());
}

Ptr<Packet>
BackwardAnt::NextHopPacket(AnthocnetRouting router) {

  if(GetHeader().GetVisitedNodes().empty()) {
    return Ptr<Packet>();
  }
  // all the mutations are already done to the header
  auto packet = Create<Packet>();
  NS_LOG_UNCOND("visited size: " << (uint32_t)(GetHeader().m_visitedSize));
  NS_LOG_UNCOND("actual size: " << GetHeader().GetVisitedNodes().size());
  NS_LOG_UNCOND("expected header size: " << GetHeader().GetSerializedSize());
  packet -> AddHeader(GetHeader());
  packet -> AddHeader(AntTypeHeader(species));

  return packet;
}


Ptr<Packet>
BackwardAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
