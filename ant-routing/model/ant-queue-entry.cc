#include "ant-queue-entry.h"

namespace ns3 {
namespace ant_routing {

AntQueueEntry::AntQueueEntry( Ptr<Ipv4Route> route, Ptr<const Packet> packet,
  const Ipv4Header& header, UnicastCallback ufcb)
   : m_route(route), m_packet(packet),m_header(header), m_unicastCallback(ufcb),
     m_sending(false), m_sendStartTime(Seconds(0)) { }

} // namespace ant_routing
} // namespace ns3
