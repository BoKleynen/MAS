#ifndef ANT_QUEUE_ENTRY_H
#define ANT_QUEUE_ENTRY_H

#include "ns3/internet-module.h"

namespace ns3 {
namespace ant_routing {

// Todo maybe add a timeout for each entry on which to discard it when it is too late?
struct AntQueueEntry {
  using UnicastCallback = Ipv4RoutingProtocol::UnicastForwardCallback;
  using ErrorCallback = Ipv4RoutingProtocol::ErrorCallback;

  AntQueueEntry( Ptr<Ipv4Route> route, Ptr<const Packet> packet,const Ipv4Header& header,  UnicastCallback ufcb);

  // bookkeeping for the router
  Ptr<Ipv4Route> m_route;
  Ptr<const Packet> m_packet;
  Ipv4Header m_header;
  UnicastCallback m_unicastCallback;

  // bookkeeping used for the sender of the queue
  bool m_sending;
  Time m_sendStartTime;
};

} // namespace ant_routing
} // namespace ns3

#endif // ANT_QUEUE_H
