#include "repair-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

RepairAnt::RepairAnt(const AntHeader& header)
  : Ant(header) { }

void
RepairAnt::Visit(AnthocnetRouting router) {

}

} // namespace ns3
} // namespace ant_routing
