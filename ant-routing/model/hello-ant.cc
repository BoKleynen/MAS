#include "hello-ant.h"
#include "ant-routing.h"
#include "neighbor-manager.h"

namespace ns3 {
namespace ant_routing {

HelloAnt::HelloAnt(Ptr<Packet> packet) {
  packet->RemoveHeader(m_header);
}


void
HelloAnt::Visit(AnthocnetRouting router) {
  NS_LOG_UNCOND("Received hello at: " << router.GetAddress() << "time: " << Simulator::Now());
  NS_LOG_UNCOND("source: " << m_header.GetSource());
  router.GetNeighborManager().HelloReceived(m_header);
  // the routingtable should contain the neighbor we just received.
  NS_LOG_UNCOND(router.GetRoutingTable().Neighbors().size());
  NS_ASSERT(router.GetRoutingTable().IsNeighbor(m_header.GetSource()));
}


} // namespace ant_routing
} // namespace ns3
