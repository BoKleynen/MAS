#include "ant-queue-entry.h"

namespace ns3 {
namespace ant_routing {

AntQueueEntry::AntQueueEntry(Ptr<const Packet> packet, Ptr<Ipv4Route> route, UnicastCallback ufcb, ErrorCallback ecb)
  : m_packet(packet), m_route(route), m_unicastCallback(ufcb), m_errorCallback(ecb) { }

} // namespace ant_routing
} // namespace ns3
