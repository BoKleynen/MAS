#include "proactive-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

ProactiveAnt::ProactiveAnt(Ipv4Address source, Ipv4Address destination) {

}


ProactiveAnt::ProactiveAnt(Ptr<Packet> packet) {
  packet->RemoveHeader(m_header);
}

void
ProactiveAnt::Visit(AnthocnetRouting router) {

}

Ptr<Packet>
ProactiveAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
