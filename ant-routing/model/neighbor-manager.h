#ifndef ANT_NEIGHBOR_MANAGER_H
#define ANT_NEIGHBOR_MANAGER_H

#include "ant-routing-table.h"

namespace ns3 {
namespace ant_routing {

// class responsible for managing the activity levels of the neighbors
// can remove/add delete entries from the Routing table based on inactivity of the neighbors
// DESIGN DECISION: This responsibillity could also have been placed on the
// HelloAnts themselves, but we felt there was need for a more 'persistent' entity that
// manages the neighbors. (for example allows for a callback mechanism)
class NeighborManager {
public:

  using FailureDetectorFactoryFunction = std::function<std::shared_ptr<NeighborFailureDetector>(Neighbor)>;

  // construction and destruction
  NeighborManager();
  NeighborManager(const AntRoutingTable& routingTable,const AntNetDevice& device);
  virtual ~NeighborManager();

  void HelloReceived(const AntHeader& Header);

  AntRoutingTable RoutingTable();
  void RoutingTable(AntRoutingTable table);

  AntNetDevice Device();
  void Device(AntNetDevice device);

  static Time HelloInterval();
  static void HelloInterval(Time interval);

private:
  // pimpl struct
  struct NeighborManagerImpl;

  NeighborManager(std::shared_ptr<NeighborManagerImpl> impl);

  Neighbor AddNeighbor(Ipv4Address address);
  void HandleNeighborFailure(Neighbor neighbor);

  // configuration of the
  static Time s_helloInterval;
  static Time s_missingInterval;
  static FailureDetectorFactoryFunction s_defaultFailureDetectorFactory;

  // performs a checkup on the neighbors that are managed
  void CheckOnNeighbors();

  std::shared_ptr<NeighborManagerImpl> m_impl;
};
}
}

#endif
