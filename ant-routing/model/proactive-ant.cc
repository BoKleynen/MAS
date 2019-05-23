#include "proactive-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

ProactiveAnt::ProactiveAnt(const AntHeader& header)
  : Ant(header) {

}

void ProactiveAnt::Visit(AnthocnetRouting router) {

}

} // namespace ant_routing
} // namespace ns3
