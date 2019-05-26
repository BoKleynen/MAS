#ifndef REACTIVE_QUEUE_H
#define REACTIVE_QUEUE_H

#include "send-queue-entry.h"

#include <memory>

namespace ns3 {
namespace ant_routing {

class AnthocnetRouting;
class AntNetDevice;
template<typename T>
class AntQueenImpl;
class ReactiveAnt;

using ReactiveQueen = AntQueenImpl<ReactiveAnt>;

template<>
class AntQueenImpl<ReactiveAnt>;
// Queue used for pending packets that could not be sent because there was no
// entry in the routing table (unicast packets only)
class ReactiveQueue {
public:
  ReactiveQueue();
  virtual ~ReactiveQueue();

  void Submit(std::shared_ptr<UnicastQueueEntry> entry, AnthocnetRouting router);
  SendQueueEntries EntryAddedFor(Ipv4Address destination);

  static void TimeoutInterval(Time timeout);
  static Time TimeoutInterval();

private:
  struct ReactiveQueueEntry;
  struct ReactiveQueueImpl;

  using PendingQueue = std::list<std::shared_ptr<ReactiveQueueEntry>>;
  using QueueMapType = std::map<Ipv4Address, std::shared_ptr<PendingQueue>>;

  // purge all the old entries from the queue
  void PrugeQueue(Ipv4Address address);
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

} // namespace ant_routing
} // namespace ns3

#endif // REACTIVE_QUEUE_H
