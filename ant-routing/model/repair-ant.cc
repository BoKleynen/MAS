#include "repair-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

RepairAnt::RepairAnt(Ptr<Packet> packet) { }

void
RepairAnt::Visit(AnthocnetRouting router) {

}

Ptr<Packet>
RepairAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ns3
} // namespace ant_routing
