#include "neighbor-manager.h"

namespace ns3 {
namespace ant_routing {

// definition of static entities -----------------------------------------------
Time NeighborManager::s_helloInterval = MilliSeconds(1000);

Time
NeighborManager::HelloInterval() {
  return s_helloInterval;
}

void
NeighborManager::HelloInterval(Time interval) {
  s_helloInterval = interval;
}


// definition of NeighborManager -----------------------------------------------

FailureDetectorFactoryFunction NeighborManager::s_defaultFailureDetectorFactory
  = [] (Neighbor neighbor) -> std::shared_ptr<NeighborFailureDetector>{
  // TODO tweak parameters?
  return MakeFailureDetector<SimpleFailureDetector>(neighbor);
};

struct NeighborManager::NeighborManagerImpl {
  NeighborManagerImpl();
  NeighborManagerImpl( const AntRoutingTable& routingTable,NeighborFactoryFunction neighborFactor, LinkFailureCallback failureCallback, FailureDetectorFactoryFunction failureDetectorFactory);
  AntRoutingTable m_routingTable; // routing table to use
  AntNetDevice m_device; // device used by all the neighbors to send data
  NeighborFactoryFunction m_neighborFactory;
  LinkFailureCallback m_failureCallback;
  FailureDetectorFactoryFunction m_failureDetectorFactory; // to create failure detectors
};

NeighborManager::NeighborManagerImpl::NeighborManagerImpl()
  : NeighborManagerImpl(AntRoutingTable(), NeighborFactoryFunction() ,LinkFailureCallback(), s_defaultFailureDetectorFactory) {

}

NeighborManager::NeighborManagerImpl::NeighborManagerImpl(
  const AntRoutingTable& routingTable,
  NeighborFactoryFunction neighborFactory,
  LinkFailureCallback failureCallback,
  FailureDetectorFactoryFunction failureDetectorFactory)
    : m_routingTable(routingTable),
      m_neighborFactory(neighborFactory),
      m_failureCallback(failureCallback),
      m_failureDetectorFactory(failureDetectorFactory)
{

}

NeighborManager::NeighborManager(const AntRoutingTable& routingTable, NeighborFactoryFunction neighborFactory, LinkFailureCallback failureCallback)
  : m_impl(std::make_shared<NeighborManagerImpl>(routingTable, neighborFactory, failureCallback, s_defaultFailureDetectorFactory)){

}

NeighborManager::NeighborManager() :
 m_impl(std::make_shared<NeighborManagerImpl>()) {

}

NeighborManager::NeighborManager(std::shared_ptr<NeighborManagerImpl> impl)
  : m_impl(impl) {

}

NeighborManager::~NeighborManager() {}

void
NeighborManager::HelloReceived(const HelloHeader& header) {
  auto sender = header.GetSource();
  auto optNeighbor = m_impl -> m_routingTable.GetNeighbor(sender);
  auto neighbor = optNeighbor.Get();

  NS_LOG_UNCOND("Received hello at neighbor manager, neighbor present? " << optNeighbor.IsValid());

  if(!optNeighbor.IsValid()) {
    neighbor = AddNeighbor(sender);
  }
  neighbor.FailureDetector() -> HelloReceived(header);
}

Neighbor
NeighborManager::AddNeighbor(Ipv4Address address) {
  Neighbor neighbor = m_impl -> m_neighborFactory(address);
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
  // add the neighbor to the routing table
  m_impl -> m_routingTable.AddNeighbor(neighbor);

  return neighbor;
}

void
NeighborManager::HandleNeighborFailure(const Neighbor& neighbor) {

  auto alternatives = m_impl -> m_routingTable.BestAlternativesFor(neighbor);
  m_impl -> m_routingTable.RemoveNeighbor(neighbor);

  std::vector<LinkFailureNotification::Message> messages;

  for(auto altIt = alternatives.begin(); altIt != alternatives.end(); altIt++) {
    LinkFailureNotification::Message message;
    message.dest = altIt -> m_destination;
    message.bestTimeEstimate = altIt -> m_pheromone.TimeEstimate();
    message.bestHopEstimate = altIt -> m_pheromone.HopCount();
    messages.push_back(message);
  }

  m_impl -> m_failureCallback(messages);
}

AntRoutingTable
NeighborManager::RoutingTable() {
  return m_impl -> m_routingTable;
}

void
NeighborManager::RoutingTable(AntRoutingTable table) {
  m_impl -> m_routingTable = table;
}


NeighborFactoryFunction NeighborManager::NeighborFactory() {
  return m_impl -> m_neighborFactory;
}

void NeighborManager::NeighborFactory(NeighborFactoryFunction neighborFactory) {
  m_impl -> m_neighborFactory = neighborFactory;
}

LinkFailureCallback NeighborManager::FailureCallback() {
  return m_impl -> m_failureCallback;
}

void NeighborManager::FailureCallback(LinkFailureCallback failureCallback) {
  m_impl -> m_failureCallback = failureCallback;
}

FailureDetectorFactoryFunction NeighborManager::FailureDectectorFactory() {
  return m_impl -> m_failureDetectorFactory;
}

void NeighborManager::FailureDetectorFactory(FailureDetectorFactoryFunction failureDetectorFactory) {
  m_impl -> m_failureDetectorFactory = failureDetectorFactory;
}

} // namespace ant_routing
} // namespace ns3
