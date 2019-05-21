#include "neighbor.h"
#include "ns3/wifi-module.h"

namespace ns3 {
namespace ant_routing {

// Neighbor definition ---------------------------------------------------------

//TODO: do we want m_addr outside of the neighbor data for faster construction?
struct Neighbor::NeighborImpl {
  NeighborImpl();
  NeighborImpl(Ipv4Address addr, Ptr<NetDevice> device);

  Ipv4Address m_addr;
  Ptr<NetDevice> m_device;

};

Neighbor::NeighborImpl::NeighborImpl()
  : NeighborImpl(Ipv4Address(), Ptr<NetDevice>()) { }

Neighbor::NeighborImpl::NeighborImpl(Ipv4Address addr, Ptr<NetDevice> device)
  :m_addr(addr), m_device(device) { }

Neighbor::Neighbor() : Neighbor(Ipv4Address(), Ptr<NetDevice>()) { }

Neighbor::Neighbor(Ipv4Address addr, Ptr<NetDevice> device) : m_impl(std::make_shared<NeighborImpl>(addr, device)) { }

Neighbor::~Neighbor() { }

Ptr<Ipv4Route>
Neighbor::CreateRoute(Ipv4Address source, Ipv4Address destination) {
  auto route = Create<Ipv4Route>();
  route->SetDestination(destination);
  route->SetSource(source);
  route->SetGateway(this->Address());
  route->SetOutputDevice(this->Device());

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

const Ptr<NetDevice>
Neighbor::Device() const {
  return m_impl->m_device;
}

void
Neighbor::Device(Ptr<NetDevice> device) {
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

bool operator<(const Neighbor& lhs, const Neighbor& rhs) {
  return lhs.Address() < rhs.Address();
}

std::ostream& operator<<(std::ostream& os, const Neighbor& nb) {
  os << "Neighbor { address: " << nb.Address() << "}";
  return os;
}


} // namespace ant_routing
} // namespace ns3
