#ifndef ANT_QUEUE_ENTRY_H
#define ANT_QUEUE_ENTRY_H

#include "ns3/internet-module.h"

namespace ns3 {
namespace ant_routing {

// Todo maybe add a timeout for each entry on which to discard it when it is too late?
struct AntQueueEntry {
  using UnicastCallback = Ipv4RoutingProtocol::UnicastForwardCallback;
  using ErrorCallback = Ipv4RoutingProtocol::ErrorCallback;

  AntQueueEntry(Ptr<const Packet> packet, Ptr<Ipv4Route> route, UnicastCallback ufcb, ErrorCallback ecb);

  Ptr<const Packet> m_packet;
  Ptr<Ipv4Route> m_route;
  UnicastCallback m_unicastCallback;
  ErrorCallback m_errorCallback;
};

} // namespace ant_routing
} // namespace ns3

#endif // ANT_QUEUE_H
