#include "forward-ant.h"
#include "ant-routing.h"
#include "ant-routing-table.h"
#include "ant-netdevice.h"
#include "ant-hill.h"
#include "backward-ant.h"
#include "neighbor-manager.h"

namespace ns3 {
namespace ant_routing {

ForwardAnt::ForwardAnt(AntHeader header)
  : m_header(header) {

  }

bool
ForwardAnt::HandleAtDestination(AnthocnetRouting router) {
  NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now().GetSeconds() << "Forward ant: handling at destination");

  // only continue if we're at the destination
  if(m_header.GetDestination() != router.GetAddress()) {
    return false;
  }
  m_header.AddVisitedNode(router.GetAddress());

  auto bwAnt = router.GetAntHill().Get<BackwardQueen>()->CreateFromForwardAnt(m_header);
  bwAnt -> Visit(router);

  return true;
}

bool
ForwardAnt::HandleUnicast(AnthocnetRouting router) {
  NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now().GetSeconds() <<"Forward ant: handling unicast");
  auto routingTable = router.GetRoutingTable();
  if(!routingTable.HasPheromoneEntryFor(m_header.GetDestination())) {
    return false;
  }

  auto optNeighbor = routingTable.RouteAnt(m_header);
  if(!optNeighbor.IsValid()) {
    return false;
  }
  auto neighbor = optNeighbor.Get();

  auto packet = NextHopPacket(router);

  UnicastPacket(packet, router, neighbor);
  return true;
}


Ptr<Packet>
ForwardAnt::NextHopPacket(AnthocnetRouting router) {
  NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now().GetSeconds() << "forward ant: handling next hop");
  AntHeader antHeader(m_header);
  antHeader.AddVisitedNode(router.GetAddress());
  antHeader.m_hopCount++;
  auto device = router.GetDevice();
  antHeader.m_timeEstimate = (device.QueueSize() + 1)*device.SendingTimeEst();
  auto packet = Create<Packet>();
  packet -> AddHeader(antHeader);
  packet -> AddHeader(AntTypeHeader(GetSpecies()));

  return packet;
}


bool
ForwardAnt::DetectLoop(Ipv4Address currentAddr) {
  for(auto addr: m_header.m_visitedNodes) {
    if(addr == currentAddr) {
      return true;
    }
  }

  return false;
}


}
}
