/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant-routing.h"
#include "ant-hill.h"
#include "ant.h"
#include "backward-ant.h"
#include "proactive-ant.h"
#include "reactive-ant.h"
#include "repair-ant.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AnthocnetRoutingProtocol");

namespace ant_routing {


struct AnthocnetRouting::AnthocnetImpl {

  AnthocnetImpl();

  AntHill m_antHill;
  // the address of the wifi interface attached
  Ipv4InterfaceAddress m_ifAddress;
  // the socket used for unicast messages in UDP
  Ptr<Socket> m_socket;
  // broadcast socket used for unicast messages in UDP
  Ptr<Socket> m_broadcastSocket;
  // the loopback device, for funneling all the packets coming from loopback to
  // the forwarding queue;
  Ptr<NetDevice> m_loopback;
  // ipv4 stack to use
  Ptr<Ipv4> m_ipv4;
};

AnthocnetRouting::AnthocnetImpl::AnthocnetImpl()
  : m_antHill(AntHill()),
    m_ifAddress(Ipv4InterfaceAddress()),
    m_socket(Ptr<Socket>()),
    m_broadcastSocket(Ptr<Socket>()),
    m_loopback(Ptr<NetDevice>()),
    m_ipv4(Ptr<Ipv4>()) {

  m_antHill.AddQueen(std::make_shared<BackwardQueen>());
  m_antHill.AddQueen(std::make_shared<ProactiveQueen>());
  m_antHill.AddQueen(std::make_shared<ReactiveQueen>());
  m_antHill.AddQueen(std::make_shared<RepairQueen>());
}


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

AnthocnetRouting::AnthocnetRouting() : m_impl(std::make_shared<AnthocnetImpl>()){ }

AnthocnetRouting::~AnthocnetRouting() { }

// we route all the output back to the device such that it becomes
// ingress traffic and be queued.
Ptr<Ipv4Route> AnthocnetRouting::RouteOutput (Ptr<Packet> p,
                          const Ipv4Header &header, Ptr<NetDevice> oif,
                          Socket::SocketErrno &sockerr)  {
  NS_ASSERT(m_impl -> m_loopback != 0);
  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetDestination(header.GetDestination());
  auto sourceAddr = !oif ? m_impl->m_ifAddress.GetLocal() : GetAddressOf(oif);
  route->SetSource(sourceAddr);
  route->SetGateway(Ipv4Address(localhost));
  route->SetOutputDevice(m_impl -> m_loopback);
  return route;
}

bool AnthocnetRouting::RouteInput  (Ptr<const Packet> p,
                          const Ipv4Header &header, Ptr<const NetDevice> idev,
                          UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                          LocalDeliverCallback lcb, ErrorCallback ecb) {
  return false;
}

void AnthocnetRouting::NotifyInterfaceUp (uint32_t interface) {
  NS_LOG_UNCOND("Interface up: " << interface);

  Ptr<Ipv4L3Protocol> ipv4l3 = m_impl -> m_ipv4 -> GetObject<Ipv4L3Protocol> ();
  if(!ipv4l3->IsUp(interface)) {
    NS_LOG_UNCOND("interface is down");
    return;
  }

  if(ipv4l3 -> GetNAddresses(interface) != 1) {
    NS_LOG_UNCOND("Anthocnet does not work with more then one address per interface.");
    return;
  }

  Ipv4InterfaceAddress ifAddress = ipv4l3 -> GetAddress(interface, 0);

  NS_LOG_UNCOND("Added: "  << ifAddress);

  if(ifAddress.GetLocal() == Ipv4Address(localhost)) {
    return;
  }

  m_impl -> m_ifAddress = ifAddress;

  // socket for unicast traffic
  auto socket = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
  NS_ASSERT (socket != 0);
  //TODO bind to the anthocnet application
  socket -> BindToNetDevice(ipv4l3->GetNetDevice(interface));
  socket -> Bind(InetSocketAddress(ifAddress.GetLocal(), ANTHOCNET_PORT));
  socket -> SetAllowBroadcast(false);

  m_impl -> m_socket = socket;

  //broadcast socket:
  auto broadcastSocket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId());
  NS_ASSERT(broadcastSocket != 0);
  //TODO set anthocnet application
  broadcastSocket -> BindToNetDevice(ipv4l3->GetNetDevice(interface));
  broadcastSocket -> Bind(InetSocketAddress(ifAddress.GetBroadcast(), ANTHOCNET_PORT));
  broadcastSocket -> SetAllowBroadcast(true);
  broadcastSocket -> SetIpRecvTtl(true);

  m_impl -> m_broadcastSocket = broadcastSocket;

  // TODO add the device associated with the new device to the neighbor table
}

void AnthocnetRouting::NotifyInterfaceDown (uint32_t interface) {
  NS_LOG_UNCOND("Interface down: " << interface);
}

void AnthocnetRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  NS_LOG_UNCOND("Added address: " << address << " to interface: " << interface);

  // should not happen...

}

void AnthocnetRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  NS_LOG_UNCOND("Removed address: " << address << " from interface: " << interface);
}

void AnthocnetRouting::SetIpv4 (Ptr<Ipv4> ipv4) {
  m_impl -> m_ipv4 = ipv4;
  // expected conditions of the ipv4 passed to routing protocol
  NS_ASSERT(ipv4->GetNInterfaces () == 1 && ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address (localhost));
  m_impl -> m_loopback = ipv4 -> GetNetDevice(0);
}

void AnthocnetRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {

}

Ipv4Address
AnthocnetRouting::GetAddressOf(Ptr<NetDevice> device) {
  auto interface = m_impl -> m_ipv4 -> GetInterfaceForDevice(device);
  auto ifAddress = m_impl -> m_ipv4 -> GetAddress(interface, 0);
  return ifAddress.GetLocal();
}

void
AnthocnetRouting::ReceiveAnt(Ptr<Socket> socket) {
  Address sourceAddress;
  Ptr<Packet> packet =  socket -> RecvFrom (sourceAddress);
  // InetSockAddress inetSourceAddr = InetSockAddress::ConvertFrom(sourceAddr);
  // Ipv4Address sender = inetSourceAddr.GetIpv4();
  // Ipv4Address receiver = m_impl -> m_ifAddress.GetLocal();
  AntHeader antHeader;
  packet->RemoveHeader(antHeader);
  // TODO integrity check? see if we received garbage?

  // TODO add methods to check if ttl is right etc, where are we going to place this
  // responsibility?
  auto ant = m_impl->m_antHill.CreateFrom(antHeader);
  if( ant == nullptr) {
    NS_LOG_WARN("Broken package received at: " << m_impl -> m_ifAddress << packet);
    return;
  }
  ant -> Visit(*this);
}

} // namespace ant_routing
} // namespace ns3
