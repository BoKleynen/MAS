#include "proactive-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

double ProactiveAnt::s_broadcastProbability = 0.10;
uint8_t ProactiveAnt::s_maxBroadcastCount = 2;

ProactiveAnt::ProactiveAnt(Ipv4Address source, Ipv4Address destination)
  : ForwardAnt(AntHeader()){

  m_header.m_source = source;
  m_header.m_dst = destination;
}


ProactiveAnt::ProactiveAnt(Ptr<Packet> packet)
  : ForwardAnt(AntHeader()) {
  packet->RemoveHeader(m_header);
}

void
ProactiveAnt::Visit(AnthocnetRouting router) {

  if(DetectLoop(router.GetAddress())) {
    NS_LOG_UNCOND("Loop detected");
    return;
  }

  // first handle the case taht at destination
  if(HandleAtDestination(router)) {
    return;
  }

  // first we may broadcast the ant (probability);
  if(HandleProbabilisticBroadcast(router)){
    return;
  }

  // second we must check that we have unicast path;
  if(HandleUnicast(router)) {
    return;
  }
  // if then we still do not have a unicast path, we broadcast the ant;
  if(HandleNoEntryBroadcast(router)) {
    return;
  }

  // case that there is no path to the destination at the current node
  // and the amount of broadcasts have been exhausted
}

bool
ProactiveAnt::HandleProbabilisticBroadcast(AnthocnetRouting router) {
  if( m_header.m_broadcastCount >= MaxBroadcastCount()
        || GetRand() > BroadcastProbability() ) {
    return false;
  }

  NS_LOG_UNCOND(router.GetAddress() << "@"<< Simulator::Now() << "- Broadcasted packet at proactive ant");

  m_header.m_broadcastCount++;
  auto packet = NextHopPacket(router);
  BroadcastPacket(packet, router);
  return true;
}

bool
ProactiveAnt::HandleNoEntryBroadcast(AnthocnetRouting router) {
  if (m_header.m_broadcastCount >= MaxBroadcastCount()) {
    NS_LOG_UNCOND(router.GetAddress() << "@"<< Simulator::Now() << "- Ran out of broadcasts at Proactive ant!");

    return false; // we have run out of broadcasts
  }

  m_header.m_broadcastCount++;
  auto packet = NextHopPacket(router);
  BroadcastPacket(packet, router);
  return true;

}

double
ProactiveAnt::BroadcastProbability() {
  return s_broadcastProbability;
}
void
ProactiveAnt::BroadcastProbability(double probability){
  if(probability > 1.0 || probability < 0.0) {
    return; // invalid range
  }

  s_broadcastProbability = probability;
}

uint8_t
ProactiveAnt::MaxBroadcastCount() {
  return s_maxBroadcastCount;
}
void
ProactiveAnt::MaxBroadcastCount(uint8_t count){
  s_maxBroadcastCount = count;
}


Ptr<Packet>
ProactiveAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
