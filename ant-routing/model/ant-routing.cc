/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

  //const std::string AnthocnetRouting::TYPENAME = "ns3::ant_routing::AnthocnetRouting";

  const uint32_t AnthocnetRouting::ANTHOCNET_PORT = 1012;

  /**
   * register the object id
   */
  TypeId AnthocnetRouting::GetTypeId (void) {
    static TypeId tid = TypeId("ns3::ant_routing::AnthocnetRouting")
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
  }

  void AnthocnetRouting::NotifyInterfaceDown (uint32_t interface) {
  }

  void AnthocnetRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  }

  void AnthocnetRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  }

  void AnthocnetRouting::SetIpv4 (Ptr<Ipv4> ipv4) {
  }

  void AnthocnetRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {

  }
}
}
