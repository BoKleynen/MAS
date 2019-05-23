/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

// ensures that the object is already registered...  NS stuff
// do not remove unless you want to break things.
NS_OBJECT_ENSURE_REGISTERED (AnthocnetRouting);

/**
 * register the object id
 */
TypeId AnthocnetRouting::GetTypeId (void) {
  static TypeId tid = TypeId(TYPENAME)
  .SetParent<Ipv4RoutingProtocol>()
  .SetGroupName("AntRouting")
  .AddConstructor<AnthocnetRouting> ();

  // TODO: Add usefull attributes for experiments

  return tid;
}

Ptr<Ipv4Route> AnthocnetRouting::RouteOutput (Ptr<Packet> p,
                          const Ipv4Header &header, Ptr<NetDevice> oif,
                          Socket::SocketErrno &sockerr)  {
  return Ptr<Ipv4Route>();
}

bool AnthocnetRouting::RouteInput  (Ptr<const Packet> p,
                          const Ipv4Header &header, Ptr<const NetDevice> idev,
                          UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                          LocalDeliverCallback lcb, ErrorCallback ecb) {
  return false;
}

void AnthocnetRouting::NotifyInterfaceUp (uint32_t interface) {
  NS_LOG_UNCOND("Interface up: " << interface);
}

void AnthocnetRouting::NotifyInterfaceDown (uint32_t interface) {
  NS_LOG_UNCOND("Interface down: " << interface);
}

void AnthocnetRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  NS_LOG_UNCOND("Added address: " << address << " to interface: " << interface);
}

void AnthocnetRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  NS_LOG_UNCOND("Removed address: " << address << " from interface: " << interface);
}

void AnthocnetRouting::SetIpv4 (Ptr<Ipv4> ipv4) {
    m_ipv4 = ipv4;
}

void AnthocnetRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {

}
}
}
