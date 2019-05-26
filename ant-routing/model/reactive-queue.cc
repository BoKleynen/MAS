#include "reactive-queue.h"
#include "ant-routing.h"
#include "ant-hill.h"
#include "reactive-ant.h"

namespace ns3 {
namespace ant_routing {

// ReactiveQueue helper classes ------------------------------------------------

struct ReactiveQueue::ReactiveQueueEntry {
  ReactiveQueueEntry(std::shared_ptr<UnicastQueueEntry> entry);

  Ipv4Address GetSource();
  Ipv4Address GetDestination();
  Time        GetSubmissionTime();
  std::shared_ptr<UnicastQueueEntry> GetUnicastEntry();

  Time m_submissionTime;
  std::shared_ptr<UnicastQueueEntry> m_unicastEntry;
};

ReactiveQueue::ReactiveQueueEntry::ReactiveQueueEntry(std::shared_ptr<UnicastQueueEntry> entry)
  : m_submissionTime(Simulator::Now()), m_unicastEntry(entry) { }

Ipv4Address
ReactiveQueue::ReactiveQueueEntry::GetSource() {
  return m_unicastEntry->GetHeader().GetSource();
}
Ipv4Address
ReactiveQueue::ReactiveQueueEntry::GetDestination() {
  return m_unicastEntry->GetHeader().GetDestination();
}
Time
ReactiveQueue::ReactiveQueueEntry::GetSubmissionTime() {
  return m_submissionTime;
}

std::shared_ptr<UnicastQueueEntry>
ReactiveQueue::ReactiveQueueEntry::GetUnicastEntry() {
  return std::shared_ptr<UnicastQueueEntry>();
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
ReactiveQueue::Submit(std::shared_ptr<UnicastQueueEntry> entry, AnthocnetRouting router) {
  auto queueEntry = std::make_shared<ReactiveQueueEntry>(entry);
  auto source = queueEntry -> GetSource();
  auto dest   = queueEntry -> GetDestination();
  auto queen = router.GetAntHill().Get<ReactiveQueen>();
  auto ant = queen->CreateNew(source, dest);

  // place the entry in the queue
  if(!HasQueue(dest)) {
    m_impl -> m_queueMap[dest] = std::make_shared<PendingQueue>();
  }
  GetPendingQueue(dest)->push_back(queueEntry);
  PrugeQueue(dest);

  ant -> Visit(router);
}

SendQueueEntries
ReactiveQueue::EntryAddedFor(Ipv4Address destination) {
  // pruge all the out to date entries
  PrugeQueue(destination);
  // transfer all the pending queue entries

  if(!HasEntries(destination)) {
    return SendQueueEntries();
  }

  SendQueueEntries entries;
  auto pendingQueue = GetPendingQueue(destination);
  for(auto entryIt = pendingQueue->begin(); entryIt != pendingQueue->end(); entryIt++) {
    entries.push_back((*entryIt) -> GetUnicastEntry());
  }

  return entries;
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

} // namespace ant_routing
} // namespace ns3
