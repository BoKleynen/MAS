#include "reactive-queue.h"
#include "ant-routing.h"
#include "ant-hill.h"
#include "reactive-ant.h"
#include "ant-routing-table.h"
#include "ns3/timer.h"

namespace ns3 {
namespace ant_routing {

// ReactiveQueue helper classes ------------------------------------------------
// struct ReactiveQueue::ReactiveQueueEntry {
//
//   ReactiveQueueEntry(Ptr<const Packet> packet, const Ipv4Header& header, uint32_t ingressInterfaceIndex,
//   UnicastCallback ufcb, ErrorCallback ecb);
//
//   Ipv4Address GetSource();
//   Ipv4Address GetDestination();
//   Time        GetSubmissionTime();
//   bool        HasEvent();
//   void        SetEvent(EventId event);
//
//   Time m_submissionTime;
//   EventId m_event;
//   Ptr<const Packet> m_packet;
//   Ipv4Header m_header;
//   uint32_t m_ingressInterfaceIndex;
//   UnicastCallback m_ufcb;
//   ErrorCallback m_ecb;
// };

struct ReactiveQueue::ReactiveQueueEntry {

  ReactiveQueueEntry(Ptr<const Packet> packet, const Ipv4Header& header, uint32_t ingressInterfaceIndex,
  UnicastCallback ufcb, ErrorCallback ecb);

  Ipv4Address GetSource();
  Ipv4Address GetDestination();
  Time        GetSubmissionTime();
  Time        GetAge();
  void        SetEvent(EventId event);

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

Time
ReactiveQueue::ReactiveQueueEntry::GetAge() {
  return Simulator::Now() - m_submissionTime;
}

// DestinationQueue

struct ReactiveQueue::DestinationQueue {

  DestinationQueue();
  virtual ~DestinationQueue() = default;

  void Push(std::shared_ptr<ReactiveQueueEntry> entry);
  void Pop ();
  bool Empty();
  std::shared_ptr<ReactiveQueueEntry> Front();
  bool IsWaitingForReactiveAnt();

  std::list<std::shared_ptr<ReactiveQueueEntry>> m_queue;
  Timer m_antTimer;
};

ReactiveQueue::DestinationQueue::DestinationQueue()
  : m_queue(std::list<std::shared_ptr<ReactiveQueueEntry>>()), m_antTimer(Timer::CANCEL_ON_DESTROY) {

}

void
ReactiveQueue::DestinationQueue::Push(std::shared_ptr<ReactiveQueueEntry> entry) {
  m_queue.push_back(entry);
}

void
ReactiveQueue::DestinationQueue::Pop() {
  m_queue.pop_front();
}

std::shared_ptr<ReactiveQueue::ReactiveQueueEntry>
ReactiveQueue::DestinationQueue::Front() {
  return m_queue.front();
}

bool
ReactiveQueue::DestinationQueue::Empty() {
  return m_queue.empty();
}

bool
ReactiveQueue::DestinationQueue::IsWaitingForReactiveAnt() {
  return m_antTimer.GetDelayLeft() > Seconds(0);
}

// ReactiveQueueImpl definition ------------------------------------------------

struct ReactiveQueue::ReactiveQueueImpl {
  ReactiveQueueImpl();
  QueueMapType m_queueMap;
};

ReactiveQueue::ReactiveQueueImpl::ReactiveQueueImpl()
  : m_queueMap(QueueMapType()) { }


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

// checks if there are any entries in the queue that have been submitted
// in a certain time interval and purges them.
void
CheckOnReactiveQueue(ReactiveQueue rQueue, Ipv4Address destination) {

  NS_LOG_UNCOND("Checked on reactive queue");

  // check if there is a queue left, if not report and return
  if(!rQueue.HasEntries(destination)) {
    NS_LOG_UNCOND("Checked on Queue for reactive ant activity.");
    return;
  }

  // all the entries older than the max age must be purged
  auto maxAge = Simulator::Now() - ReactiveQueue::TimeoutInterval();
  NS_LOG_UNCOND("max age for next iteration" << maxAge);
  auto destQueue = rQueue.GetDestinationQueue(destination);

  while(!destQueue -> Empty()) {
    NS_LOG_UNCOND("Age of front: " << destQueue -> Front() -> GetAge());
    if(destQueue -> Front() -> GetAge() < maxAge) {
      break;
    };
    NS_LOG_UNCOND("Purged entry from the reactive queue: " << destination);
    destQueue -> Pop();
  }

  if(!destQueue -> Empty()) {
    destQueue -> m_antTimer.Schedule(ReactiveQueue::TimeoutInterval());
  }
}

void
ReactiveQueue::Submit(std::shared_ptr<ReactiveQueueEntry> entry, AnthocnetRouting router) {

  NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now() << "submitted packet");

  auto dest   = entry -> GetDestination();

  // place the entry in the queue
  if(!HasQueue(dest)) {
    auto destQueue = std::make_shared<DestinationQueue>();
    destQueue -> m_antTimer.SetFunction(CheckOnReactiveQueue);
    destQueue -> m_antTimer.SetArguments(*this, dest);
    m_impl -> m_queueMap[dest] = destQueue;
  }

  auto destQueue = GetDestinationQueue(dest);
  destQueue->Push(entry);

  // reactive ant is already scheduled
  if(destQueue -> IsWaitingForReactiveAnt()) {
    NS_LOG_UNCOND("Waiting for reactive Ant");
    return;
  }

  auto queen = router.GetAntHill().Get<ReactiveQueen>();
  auto reactiveAnt = queen -> CreateNew(router.GetAddress(), dest);
  reactiveAnt -> Visit(router);

  // schedule a checkup
  destQueue -> m_antTimer.Schedule(TimeoutInterval());
}

void
ReactiveQueue::EntryAddedFor(Ipv4Address destination, AnthocnetRouting router) {
  // rogue reactive ant
  if(!HasEntries(destination)) {
    return;
  }

  NS_LOG_UNCOND("Received added entry callback ------------------ ");
  NS_LOG_UNCOND(router.GetAddress() << "has pheromone value for: " << destination  << ": " << router.GetRoutingTable().HasPheromoneEntryFor(destination));

  // in case there are entries, we'll re-ingest the packets into the router
  auto destQueue = GetDestinationQueue(destination);

  while(!destQueue -> Empty()) {
    auto entry = destQueue -> Front();
    router.HandleIngressForward( entry -> m_packet, entry -> m_header,
                                 entry -> m_ingressInterfaceIndex, entry -> m_ufcb,
                                 entry -> m_ecb);
    destQueue -> Pop();
  }

  destQueue -> m_antTimer.Cancel();
}

std::shared_ptr<ReactiveQueue::DestinationQueue>
ReactiveQueue::GetDestinationQueue(Ipv4Address dest) {
  if(!HasQueue(dest)) {
    return nullptr;
  }

  return m_impl -> m_queueMap[dest];
}

bool
ReactiveQueue::HasEntries(Ipv4Address dest) {
  return HasQueue(dest) && !(m_impl -> m_queueMap[dest] -> Empty());
}

bool
ReactiveQueue::HasQueue(Ipv4Address dest) {
  return m_impl -> m_queueMap.count(dest) != 0;
}

std::shared_ptr<ReactiveQueue::ReactiveQueueEntry>
MakeReactiveQueueEntry( Ptr<const Packet> packet, const Ipv4Header& header,
                        uint32_t ingressInterfaceIndex, UnicastCallback ufcb,
                        ErrorCallback ecb) {
  return std::make_shared<ReactiveQueue::ReactiveQueueEntry>(packet, header, ingressInterfaceIndex, ufcb, ecb);
}

} // namespace ant_routing
} // namespace ns3
