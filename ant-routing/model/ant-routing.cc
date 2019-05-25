/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant-routing.h"
#include "ant-hill.h"
#include "ant.h"
#include "backward-ant.h"
#include "hello-ant.h"
#include "link-failure-ant.h"
#include "proactive-ant.h"
#include "reactive-ant.h"
#include "repair-ant.h"
#include "ant-routing-table.h"
#include "neighbor-manager.h"
#include "ns3/nstime.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AnthocnetRoutingProtocol");

namespace ant_routing {

Time AnthocnetRouting::s_helloInterval = MilliSeconds(3000);

struct AnthocnetRouting::AnthocnetImpl {

  AnthocnetImpl();

  // the sole device used for communication with the outside world
  AntNetDevice m_device;
  // the routing table used by the algorithm to route packets
  AntRoutingTable m_routingTable;
  // managing entity for all the neighbors
  NeighborManager m_neighborManager;
  // ant hill for creating ants from incoming packets
  AntHill m_antHill;
  // the address of the wifi interface attached
  Ipv4InterfaceAddress m_ifAddress;
  // Timer for the hello messages
  Timer m_helloTimer;
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
  : m_device(AntNetDevice()),
    m_routingTable(AntRoutingTable()),
    m_neighborManager(NeighborManager()),
    m_antHill(AntHill()),
    m_ifAddress(Ipv4InterfaceAddress()),
    m_helloTimer(Timer::CANCEL_ON_DESTROY),
    m_socket(Ptr<Socket>()),
    m_broadcastSocket(Ptr<Socket>()),
    m_loopback(Ptr<NetDevice>()),
    m_ipv4(Ptr<Ipv4>()) {

  // partly initialize the neighbor manager
  // other things have to be added later when specifics become available
  m_neighborManager.RoutingTable(m_routingTable);

  m_antHill.AddQueen(std::make_shared<BackwardQueen>());
  m_antHill.AddQueen(std::make_shared<ProactiveQueen>());
  m_antHill.AddQueen(std::make_shared<ReactiveQueen>());
  m_antHill.AddQueen(std::make_shared<RepairQueen>());
  m_antHill.AddQueen(std::make_shared<HelloQueen>());
  m_antHill.AddQueen(std::make_shared<LinkFailureQueen>());
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

Ipv4Address AnthocnetRouting::GetAddress() {
  return m_impl -> m_ifAddress.GetLocal();
}

NeighborManager AnthocnetRouting::GetNeighborManager() {
  return m_impl->m_neighborManager;
}

AntRoutingTable AnthocnetRouting::GetRoutingTable() {
  return m_impl -> m_routingTable;
}

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

  NS_LOG_UNCOND("routing output packet: " << p);

  return route;
}

bool AnthocnetRouting::RouteInput  (Ptr<const Packet> p,
                          const Ipv4Header &header, Ptr<const NetDevice> idev,
                          UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                          LocalDeliverCallback lcb, ErrorCallback ecb) {
  NS_ASSERT (m_impl -> m_ipv4 != 0);
  NS_ASSERT (p != 0);
  NS_ASSERT (m_impl -> m_ipv4->GetInterfaceForDevice (idev) >= 0);

  NS_LOG_UNCOND("routing input");

  // auto source = header.GetSource();
  // auto dest   = header.GetDestination();

  NS_LOG_UNCOND("ipv4 header: " << header);
  NS_LOG_UNCOND("Broadcast allowed" << m_impl -> m_broadcastSocket -> GetAllowBroadcast ());

  if (header.GetProtocol () == UdpL4Protocol::PROT_NUMBER) {
    UdpHeader udpHeader;
    p->PeekHeader (udpHeader);
    if (udpHeader.GetDestinationPort () == ANTHOCNET_PORT) {
        uint32_t iif = m_impl -> m_ipv4->GetInterfaceForDevice (idev);
        // ANTHOCNET packets sent in broadcast are already managed, only callback
        // the higher layer
        lcb(p, header, iif);
        return true;
    }
  }

  return false;

  // only need to handle this case so far

}

void AnthocnetRouting::NotifyInterfaceUp (uint32_t interface) {
  NS_LOG_UNCOND("Interface up: " << interface);

  // once we have registered the device, we do not want an extra device to be attached.
  // TODO maybe just ignore instead of stopping the simulation
  NS_ASSERT(m_impl -> m_device.Device() == Ptr<NetDevice>());

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
  m_impl -> m_device.Device(ipv4l3->GetNetDevice(interface));

  InstallSockets();
  InstallNeighborFactory();
  InstallLinkFailureCallback();

}

void AnthocnetRouting::InstallSockets() {

  //TODO bind sockets

  auto ifAddress = m_impl -> m_ifAddress;
  auto device = m_impl -> m_device.Device();

  Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
  NS_ASSERT (socket != 0);
  socket -> SetRecvCallback(MakeCallback(&AnthocnetRouting::ReceiveAnt, this));
  socket -> BindToNetDevice(device);
  socket -> Bind(InetSocketAddress(ifAddress.GetLocal(), ANTHOCNET_PORT));
  socket -> SetAllowBroadcast(true);
  m_impl -> m_socket = socket;

  Ptr<Socket> broadcastSocket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId());
  NS_ASSERT(broadcastSocket != 0);
  broadcastSocket -> SetRecvCallback(MakeCallback(&AnthocnetRouting::ReceiveAnt, this));
  broadcastSocket -> BindToNetDevice(device);
  broadcastSocket -> Bind(InetSocketAddress(ifAddress.GetBroadcast(), ANTHOCNET_PORT));
  broadcastSocket -> SetAllowBroadcast(true);
  broadcastSocket -> SetIpRecvTtl(true);
  m_impl -> m_broadcastSocket = broadcastSocket;
}


void AnthocnetRouting::InstallNeighborFactory() {
  auto device = m_impl -> m_device;
  m_impl -> m_neighborManager.NeighborFactory([device] (Ipv4Address addr) -> Neighbor {
    return Neighbor(addr, device);
  });
}

void AnthocnetRouting::InstallLinkFailureCallback() {
  // note: need to copy the variables, will otherwise create a
  // reference loop which in turn will lead to a memory leak
  auto broadcastSocket = m_impl -> m_broadcastSocket;
  auto ifAddress = m_impl -> m_ifAddress;
  m_impl -> m_neighborManager.FailureCallback([broadcastSocket, ifAddress] (std::vector<LinkFailureNotification::Message> messages) {
    LinkFailureNotification notification(ifAddress.GetLocal(), messages);
    Ptr<Packet> packet = Create<Packet>();
    packet -> AddHeader(notification);
    AntTypeHeader typeHeader(AntType::LinkFailureAnt);
    packet -> AddHeader(typeHeader);
    broadcastSocket -> SendTo(packet, 0, InetSocketAddress(ifAddress.GetBroadcast(), ANTHOCNET_PORT));
    NS_LOG_UNCOND("Link failure called!");
  });
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

void AnthocnetRouting::DoInitialize() {
  m_impl -> m_helloTimer.Cancel();
  m_impl -> m_helloTimer.SetFunction(&AnthocnetRouting::HelloTimerExpire, this);
  Time startTime = Time(int64x64_t(GetRand()) * GetHelloTimerInterval());
  NS_LOG_UNCOND("Start time: " <<  startTime.GetSeconds());
  m_impl -> m_helloTimer.Schedule(startTime);
  Ipv4RoutingProtocol::DoInitialize();
}

void
AnthocnetRouting::HelloTimerExpire() {
  NS_LOG_UNCOND("Trying to send hello: " << Simulator::Now().GetSeconds());

  // if the broadcast socket is not yet initialized, wait
  if(!(m_impl->m_broadcastSocket)){
    NS_LOG_UNCOND("No broadcast socket set");
    m_impl -> m_helloTimer.Schedule(s_helloInterval);
  }

  Ptr<Packet> packet = Create<Packet>();
  HelloHeader helloHeader(m_impl->m_ifAddress.GetLocal());
  packet -> AddHeader(helloHeader);
  AntTypeHeader typeHeader(AntType::HelloAnt);
  packet -> AddHeader(typeHeader);

  NS_LOG_UNCOND(" sending successful: " << m_impl -> m_broadcastSocket -> SendTo(packet, 0, InetSocketAddress(m_impl->m_ifAddress.GetBroadcast(), ANTHOCNET_PORT)));
  m_impl -> m_helloTimer.Schedule(s_helloInterval);
  NS_LOG_UNCOND("Sent hello packet from: " << m_impl -> m_ifAddress.GetLocal() << "to: " << m_impl -> m_ifAddress.GetBroadcast());
}

void
AnthocnetRouting::SetHelloTimerInterval(Time interval) {
  s_helloInterval = interval;
}

Time
AnthocnetRouting::GetHelloTimerInterval() {
  return s_helloInterval;
}

Ipv4Address
AnthocnetRouting::GetAddressOf(Ptr<NetDevice> device) {
  auto interface = m_impl -> m_ipv4 -> GetInterfaceForDevice(device);
  auto ifAddress = m_impl -> m_ipv4 -> GetAddress(interface, 0);
  return ifAddress.GetLocal();
}

void
AnthocnetRouting::ReceiveAnt(Ptr<Socket> socket) {

  NS_LOG_UNCOND("input received for anthocnet");
  Address sourceAddress;
  Ptr<Packet> packet =  socket -> RecvFrom (sourceAddress);
  // InetSockAddress inetSourceAddr = InetSockAddress::ConvertFrom(sourceAddr);
  // Ipv4Address sender = inetSourceAddr.GetIpv4();
  // Ipv4Address receiver = m_impl -> m_ifAddress.GetLocal();
  AntTypeHeader typeHeader;
  packet->RemoveHeader(typeHeader);
  // TODO integrity check? see if we received garbage?

  // TODO add methods to check if ttl is right etc, where are we going to place this
  // responsibility?
  auto ant = m_impl->m_antHill.CreateFrom(typeHeader, packet);
  if( ant == nullptr) {
    NS_LOG_WARN("Broken package received at: " << m_impl -> m_ifAddress << packet);
    return;
  }
  ant -> Visit(*this);
}

} // namespace ant_routing
} // namespace ns3
