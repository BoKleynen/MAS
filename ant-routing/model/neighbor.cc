#include "neighbor.h"
#include "ns3/wifi-module.h"

namespace ns3 {
namespace ant_routing {

// Neighbor definition ---------------------------------------------------------

//TODO: do we want m_addr outside of the neighbor data for faster construction?
struct Neighbor::NeighborImpl {
  NeighborImpl();
  NeighborImpl(Ipv4Address addr, AntNetDevice device);

  Ipv4Address m_addr; // the address of the neighbor (used to keep contact & do lookups)
  AntNetDevice m_device; // the device to use to communicate with the neighbor
  std::shared_ptr<NeighborFailureDetector> m_failureDetector; // detector for the given neighbor
};

Neighbor::NeighborImpl::NeighborImpl()
  : NeighborImpl(Ipv4Address(), Ptr<NetDevice>()) { }

Neighbor::NeighborImpl::NeighborImpl(Ipv4Address addr, AntNetDevice device)
  :m_addr(addr), m_device(device), m_failureDetector(nullptr) { }

Neighbor::Neighbor() : Neighbor(Ipv4Address(), AntNetDevice() ) { }

Neighbor::Neighbor(Ipv4Address addr, AntNetDevice device) : m_impl(std::make_shared<NeighborImpl>(addr, device)) { }

Neighbor::~Neighbor() { }

Ptr<Ipv4Route>
Neighbor::CreateRoute(Ipv4Address source, Ipv4Address destination) {
  auto route = Create<Ipv4Route>();
  route->SetDestination(destination);
  route->SetSource(source);
  route->SetGateway(this->Address());
  route->SetOutputDevice(AntDevice().Device());

  return route;
}

Ipv4Address
Neighbor::Address() const {
  return m_impl -> m_addr;
}

void
Neighbor::Address(Ipv4Address addr) {
  m_impl->m_addr = addr;
}

AntNetDevice
Neighbor::AntDevice() {
  return m_impl->m_device;
}

void
Neighbor::AntDevice(AntNetDevice device) {
  m_impl->m_device = device;
}

const std::shared_ptr<Neighbor::NeighborImpl>
Neighbor::Data() const {
  return m_impl;
}

std::shared_ptr<Neighbor::NeighborImpl>
Neighbor::Data() {
  return m_impl;
}

void
Neighbor::Data(std::shared_ptr<NeighborImpl> data) {
  m_impl = data;
}

void
Neighbor::SumbitPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet,
                       const Ipv4Header &header, UnicastCallback callback) {
  AntDevice().Submit(AntQueueEntry(route, packet, header, callback));
}

void
Neighbor::SubmitExpeditedPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet,
                                const Ipv4Header &header, UnicastCallback callback) {
  AntDevice().SubmitExpedited(AntQueueEntry(route, packet, header, callback));

}

void
Neighbor::FailureDetector(std::shared_ptr<NeighborFailureDetector> detector) {
  m_impl -> m_failureDetector = detector;
}

std::shared_ptr<NeighborFailureDetector>
Neighbor::FailureDetector() {
  return m_impl -> m_failureDetector;
}

bool operator<(const Neighbor& lhs, const Neighbor& rhs) {
  return lhs.Address() < rhs.Address();
}

bool operator==(const Neighbor& lhs, const Neighbor& rhs) {
  return !(lhs < rhs || rhs < lhs);
}

std::ostream& operator<<(std::ostream& os, const Neighbor& nb) {
  os << "Neighbor { address: " << nb.Address() << "}";
  return os;
}

// Failure detection implementation --------------------------------------------

// NeihborFailureDetector -------------------------------------------------------
const Time NeighborFailureDetector::s_defaultCheckInterval = MilliSeconds(1000);

NeighborFailureDetector::NeighborFailureDetector(Neighbor neighbor)
  : m_neighbor(neighbor), m_failureCallbacks(std::vector<FailureCallback>()),
    m_checkInterval(s_defaultCheckInterval), m_suspended(false) {
}

NeighborFailureDetector::~NeighborFailureDetector() { }

void
NeighborFailureDetector::RegisterFailureCallback(FailureCallback func) {
  m_failureCallbacks.push_back(func);
}

void
NeighborFailureDetector::ClearAllCallbacks() {
  m_failureCallbacks.clear();
}

void
NeighborFailureDetector::Start() {
  OnStart();
  // call the checkup procedure for the first time
  Checkup();
}

void
NeighborFailureDetector::OnStart() {
  return; // by default: do nothing
}

void
NeighborFailureDetector::Suspend() {
  m_suspended = true;
  OnSuspend();
}

void
NeighborFailureDetector::OnSuspend() {
  return; // by default: do nothing
}

void
NeighborFailureDetector::Resume() {
  m_suspended = false;
  OnResume();
}

void
NeighborFailureDetector::OnResume() {
  return; // by default: do nothing
}

bool
NeighborFailureDetector::IsSuspended() {
  return m_suspended;
}

Time
NeighborFailureDetector::CheckInterval() {
  return m_checkInterval;
}

void
NeighborFailureDetector::CheckInterval(Time checkInterval) {
  m_checkInterval = checkInterval;
}

void
NeighborFailureDetector::ExecuteCallbacks() {
  NS_LOG_UNCOND("Callback done ----------------------------------------------");
  for(auto callbackIt = m_failureCallbacks.begin(); callbackIt != m_failureCallbacks.end(); callbackIt++ ){
    (*callbackIt)(m_neighbor);
  }
}

void
NeighborFailureDetector::Checkup() {
  // may be that a new checkup was scheduled during the deactivation of the
  // scanner, we do not want to invoke redundant callbacks
  if(IsSuspended()) {
    return;
  }

  if(HasFailed()) {
    ExecuteCallbacks();
  }

  // schedule the next checkup
  Simulator::Schedule(m_checkInterval, &NeighborFailureDetector::Checkup, this);
}

// SimpleFailureDetector -------------------------------------------------------
const uint8_t SimpleFailureDetector::s_maxMissedDefault = 3;
const Time SimpleFailureDetector::s_defaultHelloInterval = MilliSeconds(3000);

SimpleFailureDetector::SimpleFailureDetector(Neighbor neighbor)
 : SimpleFailureDetector(neighbor, s_maxMissedDefault, s_defaultHelloInterval) {

}

SimpleFailureDetector::SimpleFailureDetector(Neighbor neighbor, uint8_t maxMissed, Time helloInterval)
  : NeighborFailureDetector(neighbor), m_maxMissed(maxMissed), m_helloInterval(helloInterval), m_latestHello(Simulator::Now()) {

}

void
SimpleFailureDetector::HelloReceived(const HelloHeader& header) {
  m_latestHello = Simulator::Now();
}

bool
SimpleFailureDetector::HasFailed() {
  NS_LOG_UNCOND("has failed call: " << (Simulator::Now() - m_latestHello).GetSeconds() << ", max missed: " << (m_maxMissed * m_helloInterval).GetSeconds());
  return Simulator::Now() - m_latestHello > m_maxMissed * m_helloInterval;
}

void
SimpleFailureDetector::OnStart() {
  m_latestHello = Simulator::Now();
}

uint8_t
SimpleFailureDetector::MaxMissed() {
  return m_maxMissed;
}

void
SimpleFailureDetector::MaxMissed(uint8_t missedCount) {
  m_maxMissed = missedCount;
}

Time
SimpleFailureDetector::HelloInterval() {
  return m_helloInterval;
}

void
SimpleFailureDetector::HelloInterval(Time interval) {
  m_helloInterval = interval;
}

} // namespace ant_routing
} // namespace ns3
