#ifndef ANT_NEIGHBOR_MANAGER_H
#define ANT_NEIGHBOR_MANAGER_H

#include "ant-routing-table.h"

namespace ns3 {
namespace ant_routing {

// functions used to decrease the dependencies between the Neighbor manager
// and the specifics of the implementation
using NeighborFactoryFunction = std::function<Neighbor(Ipv4Address)>;
using LinkFailureCallback = std::function<void(std::vector<LinkFailureNotification::Message>)>;
using FailureDetectorFactoryFunction = std::function<std::shared_ptr<NeighborFailureDetector>(Neighbor)>;

// class responsible for managing the activity levels of the neighbors
// can remove/add delete entries from the Routing table based on inactivity of the neighbors
// DESIGN DECISION: This responsibillity could also have been placed on the
// HelloAnts themselves, but we felt there was need for a more 'persistent' entity that
// manages the neighbors. (for example allows for a callback mechanism)
class NeighborManager {
public:
  // construction and destruction
  NeighborManager();
  NeighborManager(const AntRoutingTable& routingTable, NeighborFactoryFunction neighborFactory, LinkFailureCallback failureCallback);
  virtual ~NeighborManager();

  void HelloReceived(const HelloHeader& Header);

  AntRoutingTable RoutingTable();
  void RoutingTable(AntRoutingTable table);

  NeighborFactoryFunction NeighborFactory();
  void NeighborFactory(NeighborFactoryFunction neighborFactory);

  LinkFailureCallback FailureCallback();
  void FailureCallback(LinkFailureCallback failureCallback);

  FailureDetectorFactoryFunction FailureDectectorFactory();
  void FailureDetectorFactory(FailureDetectorFactoryFunction failureDetectorFactory);

  static Time HelloInterval();
  static void HelloInterval(Time interval);

private:
  // pimpl struct
  struct NeighborManagerImpl;

  NeighborManager(std::shared_ptr<NeighborManagerImpl> impl);

  Neighbor AddNeighbor(Ipv4Address address);
  void HandleNeighborFailure(const Neighbor& neighbor);

  // configuration
  static Time s_helloInterval;
  static FailureDetectorFactoryFunction s_defaultFailureDetectorFactory;

  // performs a checkup on the neighbors that are managed
  void CheckOnNeighbors();

  std::shared_ptr<NeighborManagerImpl> m_impl;
};
}
}

#endif
