#include "hello-ant.h"

namespace ns3 {
namespace ant_routing {

HelloAnt::HelloAnt(const AntHeader& header)
  : Ant(header) { }


void
HelloAnt::Visit(AnthocnetRouting router) {
  // TODO implement;
}


} // namespace ant_routing
} // namespace ns3
