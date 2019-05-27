#include "reactive-queue.h"
#include "ant-routing.h"
#include "ant-hill.h"
#include "reactive-ant.h"

namespace ns3 {
namespace ant_routing {

// ReactiveQueue helper classes ------------------------------------------------
struct ReactiveQueue::ReactiveQueueEntry {

  ReactiveQueueEntry(Ptr<const Packet> packet, const Ipv4Header& header, uint32_t ingressInterfaceIndex,
  UnicastCallback ufcb, ErrorCallback ecb);

  Ipv4Address GetSource();
  Ipv4Address GetDestination();
  Time        GetSubmissionTime();

  Time m_submissionTime;
  Ptr<const Packet> m_packet;
  Ipv4Header m_header;
  uint32_t m_ingressInterfaceIndex;
  UnicastCallback m_ufcb;
  ErrorCallback m_ecb;
};

ReactiveQueue::ReactiveQueueEntry::ReactiveQueueEntry(Ptr<const Packet> packet,
                                                      const Ipv4Header& header,
                                                      uint32_t ingressInterfaceIndex,
                                                      UnicastCallback ufcb,
                                                      ErrorCallback ecb)
  : m_submissionTime(Simulator::Now()), m_packet(packet), m_header(header),
    m_ingressInterfaceIndex(ingressInterfaceIndex), m_ufcb(ufcb), m_ecb(ecb){ }

Ipv4Address
ReactiveQueue::ReactiveQueueEntry::GetSource() {
  return m_header.GetSource();
}
Ipv4Address
ReactiveQueue::ReactiveQueueEntry::GetDestination() {
  return m_header.GetDestination();
}
Time
ReactiveQueue::ReactiveQueueEntry::GetSubmissionTime() {
  return m_submissionTime;
}

struct ReactiveQueue::ReactiveQueueImpl {
  ReactiveQueueImpl();
  QueueMapType m_queueMap;
};


ReactiveQueue::ReactiveQueueImpl::ReactiveQueueImpl()
  : m_queueMap(QueueMapType()) { }

// ReactiveQueue definition ----------------------------------------------------
// ReactiveQueue statics -------------------------------------------------------

Time ReactiveQueue::s_timeoutInterval = Seconds(10); // arbitrarilly chosen

void
ReactiveQueue::TimeoutInterval(Time timeout) {
  s_timeoutInterval = timeout;
}

Time
ReactiveQueue::TimeoutInterval() {
  return s_timeoutInterval;
}
// ReactiveQueue dynamics ------------------------------------------------------
ReactiveQueue::ReactiveQueue()
  : m_impl(std::make_shared<ReactiveQueueImpl>()) { }

ReactiveQueue::~ReactiveQueue() { }

// TODO: Check if pass by value is appropriate for an entry
void
ReactiveQueue::Submit(std::shared_ptr<ReactiveQueueEntry> entry, AnthocnetRouting router) {
  auto source = entry -> GetSource();
  auto dest   = entry -> GetDestination();
  auto queen = router.GetAntHill().Get<ReactiveQueen>();
  auto ant = queen->CreateNew(source, dest);

  // place the entry in the queue
  if(!HasQueue(dest)) {
    m_impl -> m_queueMap[dest] = std::make_shared<PendingQueue>();
  }
  GetPendingQueue(dest)->push_back(entry);
  PrugeQueue(dest);

  ant -> Visit(router);
}

void
ReactiveQueue::EntryAddedFor(Ipv4Address destination, AnthocnetRouting router) {
  // pruge all the out to date entries
  PrugeQueue(destination);

  if(!HasEntries(destination)) {
    return;
  }

  // in case there are entries, we'll re-ingest the packets into the router
  auto pendingQueue = GetPendingQueue(destination);
  for(auto entryIt = pendingQueue->begin(); entryIt != pendingQueue->end(); entryIt++) {
    auto entry = (*entryIt);
    router.HandleIngressForward( entry -> m_packet, entry -> m_header,
                                 entry -> m_ingressInterfaceIndex, entry -> m_ufcb,
                                 entry -> m_ecb);
  }
}


void
ReactiveQueue::PrugeQueue(Ipv4Address dest) {
  // if there are no entries in the queue or no queue, no intervention needed
  if(!HasEntries(dest)) {
    return;
  }

  // purge all the entries that have expired in the meantime, stop if the
  // head of the queue is fresh or there are no more entries in the queue
  Time passedTime;
  auto pendingQueue = GetPendingQueue(dest);
  do {
    pendingQueue -> pop_front();
    auto submissionTime = pendingQueue -> front() ->GetSubmissionTime();
    passedTime = Simulator::Now() - submissionTime;

  } while(HasEntries(dest) && passedTime >= TimeoutInterval());

  // if after the purge, the queues are not empty, schedule a new scan for this entry
  if(HasEntries(dest)) {
    Simulator::Schedule(TimeoutInterval() - passedTime, &ReactiveQueue::PrugeQueue, this, dest);
  }
}

bool
ReactiveQueue::HasEntries(Ipv4Address dest) {
  return HasQueue(dest) && (m_impl -> m_queueMap[dest] -> empty());
}

bool
ReactiveQueue::HasQueue(Ipv4Address dest) {
  return m_impl -> m_queueMap.count(dest) != 0;
}

std::shared_ptr<ReactiveQueue::PendingQueue>
ReactiveQueue::GetPendingQueue(Ipv4Address dest) {
  if(!HasQueue(dest)) {
    return nullptr;
  }

  return m_impl -> m_queueMap[dest];
}

std::shared_ptr<ReactiveQueue::ReactiveQueueEntry>
MakeReactiveQueueEntry( Ptr<const Packet> packet, const Ipv4Header& header,
                        uint32_t ingressInterfaceIndex, UnicastCallback ufcb,
                        ErrorCallback ecb) {
  return std::make_shared<ReactiveQueue::ReactiveQueueEntry>(packet, header, ingressInterfaceIndex, ufcb, ecb);
}

} // namespace ant_routing
} // namespace ns3
