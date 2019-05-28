/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant.h"
#include "ant-routing.h"
#include "neighbor.h"
#include "ns3/socket.h"

namespace ns3 {
namespace ant_routing{

void
Ant::BroadcastPacket(Ptr<Packet> packet, AnthocnetRouting router) {
  router.BroadcastExpedited(packet);
}

void
Ant::UnicastPacket(Ptr<Packet> packet, AnthocnetRouting router, Neighbor neighbor) {
  auto socket = router.GetUnicastSocket();
  if(socket == Ptr<Socket>()) {
    NS_LOG_UNCOND("Socket uninialized while trying to unicast a packet");
    return;
  }

  packet -> AddPacketTag(ExpeditedTag());
  socket -> SendTo(packet, 0, InetSocketAddress(neighbor.Address(), AnthocnetRouting::ANTHOCNET_PORT));
}


} // namespace ant_routing
} // namespace ns3
