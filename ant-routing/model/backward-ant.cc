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
  if(router.GetAddress() == GetHeader().GetDestination()) {
    NS_LOG_UNCOND("Launched from destination ----------------------");
    HandleAtDestination(router);
  } else if(router.GetAddress() == GetHeader().GetSource()) {
    NS_LOG_UNCOND("Arrived at source ------------------------------");
    NS_ASSERT(GetHeader().GetVisitedSize() == 2); // has at least two entries source + previous
    HandleAtSource(router);
  } else {
    NS_LOG_UNCOND("Intermediary node ------------------------------");
    UpdateDistanceMetrics(router);
    UpdateRoutingTable(router);
    ReleasePending(router);
    NextHop(router);
  }
  // first get the header up to date for updating the routing table entries

}

const AntHeader&
BackwardAnt::GetHeader() {
  return m_header;
}

void
BackwardAnt::HandleAtDestination(AnthocnetRouting router) {
  NS_ASSERT(GetHeader().GetVisitedSize() > 1); // has at least two entries source + destination
  auto nxt = m_header.m_visitedNodes[m_header.GetVisitedSize() - 2];
  auto optNeighbor = router.GetRoutingTable().GetNeighbor(nxt);

  if(!optNeighbor.IsValid()) {
    NS_LOG_UNCOND("Neighbor not available for backward ant, dropped packet");
    return;
  }

  auto packet = NextHopPacket(router);
  UnicastPacket(packet, router, optNeighbor.Get());
  NS_LOG_UNCOND(" Packet was sent for unicast---------------------");

}

void
BackwardAnt::HandleAtSource(AnthocnetRouting router) {
  NS_ASSERT(GetHeader().GetVisitedSize() > 1); // has exactly two entries: source + previous
  UpdateDistanceMetrics(router);
  UpdateRoutingTable(router);
  ReleasePending(router);
}

void
BackwardAnt::UpdateDistanceMetrics(AnthocnetRouting router) {
  auto dev = router.GetDevice();
  m_header.m_hopCount ++;
  m_header.m_timeEstimate += (dev.QueueSize() + 1) * dev.SendingTimeEst();
}

void
BackwardAnt::UpdateRoutingTable(AnthocnetRouting router) {
  if(m_header.m_visitedNodes.empty()) {
    NS_ASSERT_MSG(false, "Backward ant should never have empty visited nodes during traversal"); // should not happen
    return;
  }
  auto rTable = router.GetRoutingTable();
  NS_LOG_UNCOND("router " << router.GetAddress() << "@" << Simulator::Now().GetSeconds() << ": updating pheromone entries for destination" << m_header.m_visitedNodes.back());
  router.GetRoutingTable().UpdatePheromoneEntry(m_header.m_visitedNodes.back(),
                                                m_header.m_dst,
                                                m_header.m_timeEstimate,
                                                m_header.m_hopCount);
}

void
BackwardAnt::ReleasePending(AnthocnetRouting router) {
  auto rQueue = router.GetReactiveQueue();
  rQueue.EntryAddedFor(GetHeader().GetDestination(), router);
}

void
BackwardAnt::NextHop(AnthocnetRouting router) {

  NS_ASSERT_MSG(m_header.m_visitedNodes.size() >= 3, "There should always be an entry to pop in the backward ant");

  m_header.PopVisitedNode();

  auto nextHop = m_header.m_visitedNodes[m_header.GetVisitedSize() - 2];

  auto rTable = router.GetRoutingTable();
  auto optNeighbor = rTable.GetNeighbor(nextHop);

  if(!optNeighbor.IsValid()) {
    NS_LOG_UNCOND("Neighbor got offline, dropping packet");
    return; // neighbor got offline in the meantime
  }

  auto packet = NextHopPacket(router);
  UnicastPacket(packet, router, optNeighbor.Get());
}

Ptr<Packet>
BackwardAnt::NextHopPacket(AnthocnetRouting router) {

  if(m_header.m_visitedNodes.empty()) {
    return Ptr<Packet>();
  }
  // all the mutations are already done to the header
  auto packet = Create<Packet>();
  NS_LOG_UNCOND("visited size: " << (uint32_t)(m_header.m_visitedSize));
  NS_LOG_UNCOND("actual size: " << m_header.m_visitedNodes.size());
  NS_LOG_UNCOND("expected header size: " << m_header.GetSerializedSize());
  packet -> AddHeader(m_header);
  packet -> AddHeader(AntTypeHeader(species));

  return packet;
}


Ptr<Packet>
BackwardAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
