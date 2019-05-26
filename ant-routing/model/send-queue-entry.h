#ifndef ANT_QUEUE_ENTRY_H
#define ANT_QUEUE_ENTRY_H

#include "ns3/internet-module.h"
#include <functional>
#include <memory>

namespace ns3 {
class Socket;
namespace ant_routing {

class SendQueueEntry;
class UnicastQueueEntry;
class BroadcastQueueEntry;

using UnicastCallback = Ipv4RoutingProtocol::UnicastForwardCallback;
using SendQueueEntries = std::vector<std::shared_ptr<SendQueueEntry>>;


struct SendQueueEntry {
public:

  SendQueueEntry();

  // call operator, each queue entry should be callable (regardless of the type)
  virtual void operator()() = 0;

  bool Sending();
  void Sending(bool sending);

  Time SendStartTime();
  void SendStartTime(Time startTime);
private:
  // bookkeeping used for the sender of the queue
  bool m_sending;
  Time m_sendStartTime;
};

// Todo maybe add a timeout for each entry on which to discard it when it is too late?
struct UnicastQueueEntry : public SendQueueEntry {
public:
  UnicastQueueEntry( Ptr<Ipv4Route> route, Ptr<const Packet> packet,const Ipv4Header& header,  UnicastCallback ufcb);
  virtual void operator()() override;

  Ipv4Header GetHeader();
  void SetRoute(Ptr<Ipv4Route> route);

private:
  // bookkeeping for the router
  Ptr<Ipv4Route> m_route;
  Ptr<const Packet> m_packet;
  Ipv4Header m_header;
  UnicastCallback m_unicastCallback;
};

struct BroadcastQueueEntry : public SendQueueEntry {
public:
  BroadcastQueueEntry(Ptr<Socket> socket, Ptr<Packet> packet, uint32_t flags, InetSocketAddress sockaddr);
  virtual void operator()() override;
private:
  Ptr<Socket> m_broadcastSocket;
  Ptr<Packet> m_packet;
  uint32_t m_flags;
  InetSocketAddress m_socketAddress;
};

template<typename T, typename ...Args>
std::shared_ptr<SendQueueEntry> MakeSendQueueEntry(Args...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace ant_routing
} // namespace ns3

#endif // ANT_QUEUE_H
