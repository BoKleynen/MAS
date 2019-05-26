#include "link-failure-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {
LinkFailureAnt::LinkFailureAnt(Ptr<Packet> packet) {

}

void
LinkFailureAnt::Visit(AnthocnetRouting routing) {
  // TODO implement
  NS_LOG_UNCOND("Received link failure notification");
}

Ptr<Packet>
LinkFailureAnt::ToPacket() {
  // TODO implement
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
