#include "forward-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

TypeId ForwardAnt::GetTypeId() {
  static TypeId tid = TypeId ("ns3::ant_routing::ForwardAnt")
      .SetParent<Ant>()
      .SetGroupName("AntRouting");
      .AddContructor<FowrardAnt>();
  return tid;
}

Ptr<ForwardAnt> ForwardAnt::Clone() {
  Ptr<ForwardAnt> ant = ConstructPtr();
  ant.SetAntHeader(GetAntHeader());
}


} // namespace ant_routing
} // namespace ns3
