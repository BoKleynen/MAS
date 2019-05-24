#include "neighbor-manager.h"

namespace ns3 {
namespace ant_routing {

NeighborManager::FailureDetectorFactoryFunction NeighborManager::s_defaultFailureDetectorFactory
  = [] (Neighbor neighbor) -> std::shared_ptr<NeighborFailureDetector>{
  // TODO tweak parameters?
  return MakeFailureDetector<SimpleFailureDetector>(neighbor);
};

struct NeighborManager::NeighborManagerImpl {
  NeighborManagerImpl();
  NeighborManagerImpl( const AntRoutingTable& routingTable, const AntNetDevice& device, FailureDetectorFactoryFunction failureDetectorFactory);
  AntRoutingTable m_routingTable; // routing table to use
  AntNetDevice m_device; // device used by all the neighbors to send data
  FailureDetectorFactoryFunction m_failureDetectorFactory; // to create failure detectors
};

NeighborManager::NeighborManagerImpl::NeighborManagerImpl()
  : NeighborManagerImpl(AntRoutingTable(), AntNetDevice(), s_defaultFailureDetectorFactory) {

}

NeighborManager::NeighborManagerImpl::NeighborManagerImpl(
  const AntRoutingTable& routingTable,
  const AntNetDevice& device,
  FailureDetectorFactoryFunction failureDetectorFactory)
    : m_routingTable(routingTable),
      m_device(device),
      m_failureDetectorFactory(failureDetectorFactory){

}

NeighborManager::NeighborManager(const AntRoutingTable& routingTable, const AntNetDevice& device)
  : m_impl(std::make_shared<NeighborManagerImpl>(routingTable, device, s_defaultFailureDetectorFactory)){

}

NeighborManager::NeighborManager() :
 NeighborManager(AntRoutingTable(), AntNetDevice()) {

}

NeighborManager::NeighborManager(std::shared_ptr<NeighborManagerImpl> impl)
  : m_impl(impl) {

}

NeighborManager::~NeighborManager() {}

void
NeighborManager::HelloReceived(const AntHeader& header) {
  auto sender = header.GetOrigin();
  auto neighborPair = m_impl -> m_routingTable.GetNeighbor(sender);
  auto neighbor = neighborPair.first;

  if(!neighborPair.second) {
    neighbor = AddNeighbor(sender);
  }
  neighbor.FailureDetector() -> HelloReceived(header);
}

Neighbor
NeighborManager::AddNeighbor(Ipv4Address address) {
  Neighbor neighbor(address, m_impl -> m_device);
  auto detector = m_impl->m_failureDetectorFactory(neighbor);
  // bind the current neighbor manager to the callback
  auto impl = m_impl; // needed to make the lambda work
  detector -> RegisterFailureCallback([impl] (Neighbor neighbor) {
    // to solve the problem that 'this' got delted, regenerate 'this' from
    // the implementation pointer
    auto manager = NeighborManager(impl);
    manager.HandleNeighborFailure(neighbor);
  });

  neighbor.FailureDetector(detector);

  return neighbor;
}

void
NeighborManager::HandleNeighborFailure(Neighbor neighbor) {
  // TODO implement;
}



AntRoutingTable
NeighborManager::RoutingTable() {
  return m_impl -> m_routingTable;
}

void
NeighborManager::RoutingTable(AntRoutingTable table) {
  m_impl -> m_routingTable = table;
}

AntNetDevice
NeighborManager::Device() {
  return m_impl -> m_device;
}
void
NeighborManager::Device(AntNetDevice device) {
  m_impl -> m_device = device;
}

} // namespace ant_routing
} // namespace ns3
