#ifndef REACTIVE_QUEUE_H
#define REACTIVE_QUEUE_H

#include "send-queue-entry.h"

#include <memory>

namespace ns3 {
namespace ant_routing {

// forward declarations
class AnthocnetRouting;
class AntNetDevice;

template<typename T>
class AntQueenImpl;
class ReactiveAnt;

using ReactiveQueen = AntQueenImpl<ReactiveAnt>;
template<>
class AntQueenImpl<ReactiveAnt>;


using UnicastCallback = Ipv4RoutingProtocol::UnicastForwardCallback;
using ErrorCallback   = Ipv4RoutingProtocol::ErrorCallback;

// Queue used for pending packets that could not be sent because there was no
// entry in the routing table (unicast packets only)
class ReactiveQueue {
public:

  friend void PurgeQueue(ReactiveQueue rQueue, Ipv4Address dest);

  struct ReactiveQueueEntry;

  ReactiveQueue();
  ReactiveQueue(const ReactiveQueue& other);
  virtual ~ReactiveQueue();

  ReactiveQueue& operator=(const ReactiveQueue& other);

  void Submit(std::shared_ptr<ReactiveQueueEntry> entry, AnthocnetRouting router);
  void EntryAddedFor(Ipv4Address destination, AnthocnetRouting router);

  static void TimeoutInterval(Time timeout);
  static Time TimeoutInterval();

private:
  struct ReactiveQueueImpl;

  using PendingQueue = std::list<std::shared_ptr<ReactiveQueueEntry>>;
  using QueueMapType = std::map<Ipv4Address, std::shared_ptr<PendingQueue>>;

  // checks if there are any entries for the given destination address
  // returns true both in case the there is no queue for the address
  // or there are no pending packets in the queue for the address
  bool HasEntries(Ipv4Address dest);

  // checks if there is a queue associated with the given destination
  bool HasQueue(Ipv4Address dest);

  // getter for the queue associated with the pending queue
  // in case there is no queue for the destination returns nullptr
  // in all other cases returns the value set for the pending queue
  std::shared_ptr<PendingQueue> GetPendingQueue(Ipv4Address dest);

  // how long the entry may be in the queue before it is discarded;
  static Time s_timeoutInterval;

  std::shared_ptr<ReactiveQueueImpl> m_impl;
};

// purge all the old entries from the queue
void PurgeQueue(ReactiveQueue rQueue, Ipv4Address address);

std::shared_ptr<ReactiveQueue::ReactiveQueueEntry>
MakeReactiveQueueEntry( Ptr<const Packet> packet, const Ipv4Header& header,
                        uint32_t ingressInterfaceIndex, UnicastCallback ufcb,
                        ErrorCallback ecb);
} // namespace ant_routing
} // namespace ns3

#endif // REACTIVE_QUEUE_H
