#include "reactive-queue.h"
#include "ant-routing.h"
#include "ant-hill.h"
#include "reactive-ant.h"
#include "ant-routing-table.h"

namespace ns3 {
namespace ant_routing {

// ReactiveQueue helper classes ------------------------------------------------
struct ReactiveQueue::ReactiveQueueEntry {

  ReactiveQueueEntry(Ptr<const Packet> packet, const Ipv4Header& header, uint32_t ingressInterfaceIndex,
  UnicastCallback ufcb, ErrorCallback ecb);

  Ipv4Address GetSource();
  Ipv4Address GetDestination();
  Time        GetSubmissionTime();
  bool        HasEvent();
  void        SetEvent(EventId event);

  Time m_submissionTime;
  EventId m_event;
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
  : m_submissionTime(Simulator::Now()), m_event(EventId()), m_packet(packet), m_header(header),
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


bool
ReactiveQueue::ReactiveQueueEntry::HasEvent() {
  return m_event != EventId();
}

void
ReactiveQueue::ReactiveQueueEntry::SetEvent(EventId event) {
  m_event = event;
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

ReactiveQueue& ReactiveQueue::operator=(const ReactiveQueue& other){
  m_impl = other.m_impl;
  return *this;
}

ReactiveQueue::ReactiveQueue(const ReactiveQueue& other)
  : m_impl(other.m_impl) {
}

// TODO: Check if pass by value is appropriate for an entry
void
ReactiveQueue::Submit(std::shared_ptr<ReactiveQueueEntry> entry, AnthocnetRouting router) {

  NS_LOG_UNCOND("forwarding packet");

  auto dest   = entry -> GetDestination();
  auto queen = router.GetAntHill().Get<ReactiveQueen>();
  auto ant = queen->CreateNew(router.GetAddress(), dest);

  // place the entry in the queue
  if(!HasQueue(dest)) {
    m_impl -> m_queueMap[dest] = std::make_shared<PendingQueue>();
  }
  PurgeQueue(*this, dest);
  GetPendingQueue(dest)->push_back(entry);

  ant -> Visit(router);
}

void
ReactiveQueue::EntryAddedFor(Ipv4Address destination, AnthocnetRouting router) {
  // pruge all the out to date entries
  PurgeQueue(*this, destination);

  if(!HasEntries(destination)) {
    NS_LOG_UNCOND("Has no entries yet for the destination");
    return;
  }

  NS_LOG_UNCOND("Received added entry callback ------------------ ");
  NS_LOG_UNCOND(router.GetAddress() << "has pheromone value for: " << destination  << ": " << router.GetRoutingTable().HasPheromoneEntryFor(destination));

  // in case there are entries, we'll re-ingest the packets into the router
  auto pendingQueue = GetPendingQueue(destination);
  for(auto entryIt = pendingQueue->begin(); entryIt != pendingQueue->end(); entryIt++) {
    NS_LOG_UNCOND("sending ingress traffic");
    auto entry = (*entryIt);
    router.HandleIngressForward( entry -> m_packet, entry -> m_header,
                                 entry -> m_ingressInterfaceIndex, entry -> m_ufcb,
                                 entry -> m_ecb);
  }
  pendingQueue -> clear();
}


void
PurgeQueue(ReactiveQueue rQueue, Ipv4Address dest) {
  //
  // NS_LOG_UNCOND("Call to purge made");
  // NS_LOG_UNCOND("m_impl: " << rQueue.m_impl.get() << " ~~~~~~~~~~~~~~~~~~~~~~ ");

  // if there are no entries in the queue or no queue, no intervention needed
  if(!rQueue.HasEntries(dest)) {
    return;
  }
  NS_ASSERT(rQueue.GetPendingQueue(dest) -> size() < 10);
  NS_LOG_UNCOND("Has entries for destination ~~~~ " <<  rQueue.GetPendingQueue(dest) -> size() << " entries total");

  // purge all the entries that have expired in the meantime, stop if the
  // head of the queue is fresh or there are no more entries in the queue
  // auto pendingQueue = rQueue.GetPendingQueue(dest);
  // auto submissionTime = pendingQueue -> front() ->GetSubmissionTime();
  // Time passedTime = Simulator::Now() - submissionTime;
  NS_LOG_UNCOND("Current time: " << Simulator::Now().GetSeconds());

  // if(passedTime >= ReactiveQueue::TimeoutInterval()) {
  //   do {
  //     pendingQueue -> pop_front();
  //     submissionTime = pendingQueue -> front() ->GetSubmissionTime();
  //     passedTime = Simulator::Now() - submissionTime;
  //     NS_LOG_UNCOND("purged entry");
  //   } while(rQueue.HasEntries(dest) && passedTime >= ReactiveQueue::TimeoutInterval());
  // }
  // head of the queue is fresh or there are no more entries in the queue

  auto passedTime = [] (auto pQueue) -> Time {
    Time submissionTime = pQueue -> front() -> GetSubmissionTime();
    return Simulator::Now() - submissionTime;
  };

  auto queueHead = [] (auto pQueue) {
    return pQueue -> front();
  };

  auto pendingQueue = rQueue.GetPendingQueue(dest);
  while(rQueue.HasEntries(dest)) {
    if(passedTime(pendingQueue) < ReactiveQueue::TimeoutInterval()) {
      break;
    }
    pendingQueue -> pop_front();
    NS_LOG_UNCOND("purged entry#");
  }

  // if after the purge, the queues are not empty, schedule a new scan for this entry
  if(rQueue.HasEntries(dest) && (!queueHead(pendingQueue) -> HasEvent())){
    NS_LOG_UNCOND("scheduling next iteration in " << (ReactiveQueue::TimeoutInterval() - passedTime(pendingQueue)).GetSeconds() << " seconds");
    auto event = Simulator::Schedule((ReactiveQueue::TimeoutInterval() - passedTime(pendingQueue)), PurgeQueue, rQueue, dest);
    queueHead(pendingQueue) -> SetEvent(event);
  }
}


bool
ReactiveQueue::HasEntries(Ipv4Address dest) {
  return HasQueue(dest) && !(m_impl -> m_queueMap[dest] -> empty());
}

bool
ReactiveQueue::HasQueue(Ipv4Address dest) {
  // NS_LOG_UNCOND("nullptr impl: " << (m_impl == nullptr));
  // NS_LOG_UNCOND("empty queue map: " << (m_impl -> m_queueMap.empty()));
  // NS_LOG_UNCOND("destination address: " << dest);
  // NS_LOG_UNCOND("counts for destination: " << (m_impl -> m_queueMap.size()));
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
