#include "send-queue-entry.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SendQueueEntry");

namespace ant_routing {

SendQueueEntry::SendQueueEntry()
  : m_sending(false), m_sendStartTime(Seconds(0)) { }


bool
SendQueueEntry::Sending() {
  return m_sending;
}

void
SendQueueEntry::Sending(bool sending) {
  m_sending = sending;
}

Time
SendQueueEntry::SendStartTime() {
  return m_sendStartTime;
}

void
SendQueueEntry::SendStartTime(Time startTime) {
  m_sendStartTime = startTime;
}

// Unicast queue entry ---------------------------------------------------------
UnicastQueueEntry::UnicastQueueEntry(Ptr<Ipv4Route> route, Ptr<const Packet> packet,
  const Ipv4Header& header, UnicastCallback ufcb)
   : SendQueueEntry(), m_route(route), m_packet(packet), m_header(header), m_unicastCallback(ufcb) { }


void
UnicastQueueEntry::operator()() {
  m_unicastCallback(m_route, m_packet, m_header);
}

Ipv4Header UnicastQueueEntry::GetHeader() {
  return m_header;
}


void
UnicastQueueEntry::SetRoute(Ptr<Ipv4Route> route) {
  m_route = route;
}

// BroadcastQueueEntry definition ----------------------------------------------

BroadcastQueueEntry::BroadcastQueueEntry(Ptr<Socket> socket, Ptr<Packet> packet, uint32_t flags, InetSocketAddress sockAddr)
  : SendQueueEntry(), m_broadcastSocket(socket), m_packet(packet), m_flags(flags), m_socketAddress(sockAddr) { }

void
BroadcastQueueEntry::operator()() {
  if(!m_broadcastSocket || !m_broadcastSocket->GetAllowBroadcast()) {
    NS_LOG_WARN("Error trying to broadcast a signal on a non broadcast socket");
    return; // TODO log this?
  }

  m_broadcastSocket -> SendTo(m_packet, m_flags, m_socketAddress);
}

} // namespace ant_routing
} // namespace ns3
