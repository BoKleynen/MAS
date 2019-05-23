#include "reactive-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

ReactiveAnt::ReactiveAnt(const AntHeader& header)
  : Ant(header) {
}

void ReactiveAnt::Visit(AnthocnetRouting router){
  //TODO implement
}

} // namespace ant_routing
} // namespace ns3
