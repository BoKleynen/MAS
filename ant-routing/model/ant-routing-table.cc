#include "ant-routing-table.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ant-Routing-Table");

namespace ant_routing{


// PheromoneEntry definition ---------------------------------------------------
PheromoneEntry::PheromoneEntry() : PheromoneEntry(0, 0, Seconds(0)){ }

PheromoneEntry::PheromoneEntry(double value, uint32_t hopCount, Time timeEstimate) :
  m_value(value), m_hopCount(hopCount), m_timeEstimate(timeEstimate) { }

double
PheromoneEntry::Value() const {
  return m_value;
}

void
PheromoneEntry::Value(double value) {
  m_value = value;
}

uint32_t
PheromoneEntry::HopCount() const {
  return m_hopCount;
}

void
PheromoneEntry::HopCount(uint32_t hopCount) {
  m_hopCount = hopCount;
}

Time
PheromoneEntry::TimeEstimate() const {
  return m_timeEstimate;
}

void
PheromoneEntry::TimeEstimate(Time timeEstimate) {
  m_timeEstimate = timeEstimate;
}

std::ostream&
operator<<(std::ostream& os, const PheromoneEntry& pe) {
  os << "PheromoneEntry { value: " << pe.Value() << " timeEstimate: "<< pe.TimeEstimate() << " hopCount "<< (uint32_t)pe.HopCount() <<"}";
  return os;
}

// AlternativeRoute implementation ---------------------------------------------
AlternativeRoute::AlternativeRoute()
  : AlternativeRoute(Ipv4Address(), Neighbor(),PheromoneEntry(), false) { }

AlternativeRoute::AlternativeRoute(Ipv4Address dest, Neighbor neighbor, PheromoneEntry pheromone, bool valid)
  : m_destination(dest), m_neighbor(neighbor), m_pheromone(pheromone), m_valid(valid) { }

std::ostream& operator<<(std::ostream& os, const AlternativeRoute& ar) {
  if (ar.m_valid) {
    os << "Alternative route { dest: "<< ar.m_destination <<", neighbor: " << ar.m_neighbor << ", pheromone: "<< ar.m_pheromone << "}";
  } else {
  os << "Alternative route { dest: "<< ar.m_destination << "}";
  }
  return os;
}

// NeighborKey Implementation --------------------------------------------------
NeighborKey::NeighborKey() : m_addr(Ipv4Address()), m_neighbor (nullptr) { }
NeighborKey::NeighborKey(Neighbor neighbor) : m_addr(neighbor.Address()), m_neighbor(std::make_shared<Neighbor>(neighbor)) { }
NeighborKey::NeighborKey(Ipv4Address addr) : m_addr(addr), m_neighbor(nullptr) { }

Ipv4Address NeighborKey::Address() const {
  return m_addr;
}

Neighbor NeighborKey::Get() const {
  return static_cast<Neighbor>(*this);
}

NeighborKey::operator Neighbor() const {
  if(m_neighbor == nullptr) {
    m_neighbor = std::make_shared<Neighbor>(m_addr, Ptr<NetDevice>());
  }

  return *m_neighbor;
}

bool operator<(const NeighborKey &lhs, const NeighborKey &rhs) {
  return lhs.Address() < rhs.Address();
}

bool operator==(const NeighborKey &lhs, const NeighborKey& rhs) {
  return !(lhs < rhs || rhs < lhs);
}


// AntRoutingTable definition --------------------------------------------------
// static variables:
double AntRoutingTable::s_antBeta = 1.0;
double AntRoutingTable::s_packetBeta = 1.0;
double AntRoutingTable::s_gamma = 0.7;
Time   AntRoutingTable::s_hopTime = MilliSeconds(3);
double AntRoutingTable::s_bestEstCoeff = 0.5;

// implementation of methods
AntRoutingTable::AntRoutingTable() : m_table(std::make_shared<RoutingTableType>()) { }

// methods related to generating routes
Ptr<Ipv4Route>
AntRoutingTable::RouteTo(const Ipv4Header& ipv4h) {
  return RouteTo(ipv4h.GetSource(), ipv4h.GetDestination(), PacketBeta());
}

Ptr<Ipv4Route>
AntRoutingTable::RouteTo(const AntHeader& ah){
  return RouteTo(ah.GetSource(), ah.GetDestination(), AntBeta());
}

Ptr<Ipv4Route>
AntRoutingTable::RouteTo(Ipv4Address source, Ipv4Address dest, double beta) {
  auto optNeighbor = RouteToNeighbor(source, dest, beta);
  return optNeighbor.IsValid() ? optNeighbor.Get().CreateRoute(source, dest) : Ptr<Ipv4Route>();
}

OptNeighbor
AntRoutingTable::RoutePacket(const Ipv4Header& header) {
  return RouteToNeighbor(header.GetSource(), header.GetDestination(), PacketBeta());
}

OptNeighbor
AntRoutingTable::RouteAnt(const AntHeader& header) {
  return RouteToNeighbor(header.GetSource(), header.GetDestination(), AntBeta());
}

OptNeighbor
AntRoutingTable::RouteToNeighbor(Ipv4Address source, Ipv4Address dest, double beta) {

    // if direct neighbor, no need to evaluate the pheromone table
    auto optNeighbor = GetNeighbor(dest);
    if(optNeighbor.IsValid()) {
      return optNeighbor;
    }

    // case that no entries in the table (no neighbors) return empty pointer
    if (m_table->empty() || !HasPheromoneEntryFor(dest)) {
      return OptNeighbor();
    }

    double totalPheromone = TotalPheromone(dest, beta);
    double selectionPoint = GetRand();
    double accumulator = 0;
    auto neighbors = Neighbors();

    // loop over all entries except for the last one
    for(auto neighborIt = neighbors.begin(); neighborIt != --neighbors.end(); neighborIt++) {
      auto entryPtr = GetPheromone(neighborIt->Address(), dest);
      if (entryPtr != nullptr) {
        accumulator += (pow(entryPtr->Value(), beta) / totalPheromone);
      }
      if (selectionPoint <= accumulator) {
        return OptNeighbor(neighborIt->Get());
      }
    }

    // return the last entry in case the loop completed.
    // This seperate case is needed to deal with rounding errors in the accumulator
    return OptNeighbor((neighbors.rbegin())->Get());
}

std::vector<Ptr<Ipv4Route>>
AntRoutingTable::NoPheromoneRoutes(const AntHeader& ah) {
  Ipv4Address source = ah.GetSource();
  Ipv4Address destination = ah.GetDestination();
  std::vector<Ptr<Ipv4Route>> routes;
  auto neighbors = Neighbors();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt ++) {
    auto entryPtr = GetPheromone(neighborIt -> Address(), destination);
    if (entryPtr == nullptr) {
      auto route = neighborIt->Get().CreateRoute(source, destination);
      routes.push_back(route);
    }
  }

  return routes;
}

std::vector<Neighbor>
AntRoutingTable::NoPheromoneNeighbors(const AntHeader& header) {
  Ipv4Address dest   = header.GetDestination();
  std::vector<Neighbor> neighbors;

  for(auto neighborTableIt = m_table -> begin(); neighborTableIt != m_table -> end(); neighborTableIt++) {
    auto entryPtr = GetPheromone(neighborTableIt -> first.Address(), dest);
    if(entryPtr == nullptr) {
      auto neighbor = neighborTableIt -> first.Get();
      neighbors.push_back(neighbor);
    }
  }

  return neighbors;
}

std::vector<Ptr<Ipv4Route>>
AntRoutingTable::BroadcastRouteTo(const AntHeader& ah) {
  Ipv4Address source = ah.GetSource();
  Ipv4Address destination = ah.GetDestination();
  std::vector<Ptr<Ipv4Route>> routes;
  auto neighbors = Neighbors();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt++) {
    auto route = neighborIt->Get().CreateRoute(source, destination);
    routes.push_back(route);
  }

  return routes;
}

std::vector<Neighbor>
AntRoutingTable::BroadcastNeighbors() {
  std::vector<Neighbor> neighbors;
  for(auto neighborTableIt = m_table -> begin(); neighborTableIt != m_table->end(); neighborTableIt ++){
    neighbors.push_back(neighborTableIt->first.Get());
  }

  return neighbors;
}



// methods related to pheromone management
void
AntRoutingTable::UpdatePheromoneEntry(Ipv4Address neighbor, Ipv4Address dest, Time timeEstimate,  uint32_t hops) {
  auto neighborTablePtr = GetNeighborTable(NeighborKey(neighbor));
  if (neighborTablePtr == nullptr) {
    NS_LOG_WARN("Tried to update a next hop pheromone entry of a node that is not a neighbor.");
    return;
  }
  // create a new pheromone entry in case there isn't one yet
  if(!HasPheromoneEntryFor(neighbor, dest)) {
    neighborTablePtr->operator[](dest) = std::make_shared<PheromoneEntry>();
  }
  // update the entry in the map
  std::shared_ptr<PheromoneEntry> pheromoneEntryPtr = neighborTablePtr->operator[](dest);
  double extraPheromone = 2/(timeEstimate.GetSeconds() + hops*HopTime().GetSeconds());
  pheromoneEntryPtr->Value(s_gamma * pheromoneEntryPtr->Value() + (1 - s_gamma)*extraPheromone);
  pheromoneEntryPtr->HopCount(s_bestEstCoeff * pheromoneEntryPtr ->HopCount() + (1-s_bestEstCoeff)*hops);
  auto timeUpdate = Seconds(s_bestEstCoeff* pheromoneEntryPtr->TimeEstimate().GetSeconds() + (1-s_bestEstCoeff)*timeEstimate.GetSeconds());
  NS_LOG_UNCOND("Time updated by the backward ant: " << timeUpdate);
  pheromoneEntryPtr->TimeEstimate(timeUpdate);
}

bool
AntRoutingTable::HasPheromoneEntryFor(Ipv4Address destination) {
  auto neighbors = Neighbors();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt++) {
    auto entryPtr = GetPheromone(neighborIt -> Address(), destination);

    if(entryPtr != nullptr) {
      return true;
    }
  }

  return false;
}

bool
AntRoutingTable::HasPheromoneEntryFor(Ipv4Address source, Ipv4Address destination) {
  return GetPheromone(source, destination) != nullptr;
}

void
AntRoutingTable::DeletePheromoneEntryFor(Ipv4Address neighbor, Ipv4Address destination) {
  auto neighborTable = GetNeighborTable(neighbor);
  neighborTable -> erase(destination);
}

const std::shared_ptr<PheromoneEntry>
AntRoutingTable::GetPheromone(Ipv4Address neighbor, Ipv4Address destination) {
  auto neighborTableIt = m_table->find(neighbor);

  if (neighborTableIt == m_table->end()) {
    return nullptr;
  }

  std::shared_ptr<NeighborTable> neighborTable = neighborTableIt->second;
  auto pheromoneEntryIt = neighborTable->find(destination);

  if(pheromoneEntryIt == neighborTable->end()) {
    return nullptr;
  }

  return pheromoneEntryIt->second;

}

void
AntRoutingTable::SetPheromoneAt(Ipv4Address neighbor, Ipv4Address destination, const PheromoneEntry& entry) {
  // the neighbor table must exist, otherwise we silently ignore the call
  auto neighborTableIt = m_table->find(neighbor);

  if (neighborTableIt == m_table->end()) {
    return;
  }

  // change the entry for the destination or create a new one for the
  // given destination in case there wasn't any.
  std::shared_ptr<NeighborTable> neighborTable = neighborTableIt->second;
  (*neighborTable)[destination] = std::make_shared<PheromoneEntry>(entry);
}

double AntRoutingTable::TotalPheromone(Ipv4Address dest, double beta) {
  double totalPheromone = 0;

  for(auto nbrTableIt = m_table->begin(); nbrTableIt != m_table->end(); nbrTableIt++) {
    std::shared_ptr<NeighborTable> nbrTablePtr = nbrTableIt -> second;
    auto pheromoneIt = nbrTablePtr->find(dest);
    // if there is an entry for the destination, add the value
    if (pheromoneIt != nbrTablePtr->end()) {
      std::shared_ptr<PheromoneEntry> entry = pheromoneIt->second;
      totalPheromone += pow(entry->Value(), beta);
    }
  }

  return totalPheromone;
}


// neighbor management

void
AntRoutingTable::AddNeighbor(const Neighbor& nb) {
  (*m_table)[NeighborKey(nb)] = std::make_shared<NeighborTable>();
}
void
AntRoutingTable::RemoveNeighbor(const Neighbor& nb) {
  m_table->erase(nb);
}

bool
AntRoutingTable::HasNeighbors() {
  return !(m_table -> empty());
}

OptNeighbor AntRoutingTable::GetNeighbor(Ipv4Address addr) {
  auto neighborTableIter = m_table->find(addr);
  if (neighborTableIter != m_table->end()) {
    return OptNeighbor(neighborTableIter->first);
  }

  return OptNeighbor();
}

std::vector<AlternativeRoute>
AntRoutingTable::BestAlternativesFor(const Neighbor& neighbor) {

  std::vector<AlternativeRoute> alternatives;

  auto neighborTable = GetNeighborTable(neighbor);
  for(auto destEntryIt = neighborTable->begin(); destEntryIt != neighborTable->end(); destEntryIt++) {

    if (!IsBestEntryFor(neighbor, destEntryIt->first)) {
      continue;
    }

    auto alt = GetBestAlternativeFor(neighbor, destEntryIt->first);
    alternatives.push_back(alt);
  }

  return alternatives;
}


bool
AntRoutingTable::IsBestEntryFor(Ipv4Address neighborAddr, Ipv4Address destination) {
  auto neighborEntry = GetPheromone(neighborAddr, destination);

  if(neighborEntry == nullptr) {
    return false;
  }

  auto neighbors = Neighbors();
  double neighborValue = neighborEntry->Value();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt++) {

    if(neighborIt -> Get().Address() == neighborAddr) {
      continue;
    }

    auto entry = GetPheromone(neighborIt->Address(), destination);
    if (entry != nullptr && (entry -> Value()) > neighborValue) {
      return false;
    }
  }
  return true;
}

bool
AntRoutingTable::IsBestEntryFor(const Neighbor& neighbor, Ipv4Address destination) {
  return IsBestEntryFor(neighbor.Address(), destination);
}

AlternativeRoute
AntRoutingTable::GetBestAlternativeFor(Ipv4Address neighborAddr, Ipv4Address destination) {
  auto neighbors = Neighbors();

  AlternativeRoute bestAlt;
  bestAlt.m_destination = destination;

  for(auto neighborIt =  neighbors.begin(); neighborIt != neighbors.end(); neighborIt++) {
    if(neighborIt -> Get().Address() == neighborAddr) {
      continue;
    }

    auto entry = GetPheromone(neighborIt->Address(), destination);
    if (entry != nullptr && (entry -> Value()) > bestAlt.m_pheromone.Value()) {
      bestAlt = AlternativeRoute(destination, *neighborIt, *entry, true);
    }
  }

  return bestAlt;
}


std::vector<LinkFailureNotification::Message>
ConvertAlternatives(std::vector<AlternativeRoute> alternatives) {
  std::vector<LinkFailureNotification::Message> messages;
  for(auto altIt = alternatives.begin(); altIt != alternatives.end(); altIt++) {

    LinkFailureNotification::Message message;
    message.dest = altIt -> m_destination;

    if(altIt -> m_valid) {
      message.bestTimeEstimate = altIt ->m_pheromone.TimeEstimate();
      message.bestHopEstimate = altIt ->m_pheromone.HopCount();
    }
    message.SetValidEstimates(altIt -> m_valid);
    messages.push_back(message);
  }
  return messages;
}


AlternativeRoute
AntRoutingTable::GetBestAlternativeFor(const Neighbor& neighbor, Ipv4Address destination) {
  return GetBestAlternativeFor(neighbor.Address(), destination);
}

bool AntRoutingTable::IsNeighbor(Ipv4Address addr) {
  return IsNeighbor(NeighborKey(addr));
}

bool AntRoutingTable::IsNeighbor(Neighbor neighbor) {
  auto neighborTableIter = m_table->find(neighbor);
  return neighborTableIter != m_table->end();
}


// returns a vector containing all the neighbors registered in the
// routing table.
std::vector<NeighborKey>
AntRoutingTable::Neighbors() {
  std::vector<NeighborKey> neighbors;
  for (auto neighborTableIt = m_table->begin(); neighborTableIt != m_table->end(); neighborTableIt ++ ) {
    neighbors.push_back(neighborTableIt->first);
  }

  return neighbors;
}

std::shared_ptr<AntRoutingTable::NeighborTable> AntRoutingTable::GetNeighborTable(NeighborKey neighbor) {
  auto neighborTableIt = m_table->find(neighbor);
  return neighborTableIt != m_table->end() ? neighborTableIt->second : std::make_shared<NeighborTable>();
}


double
AntRoutingTable::AntBeta() {
  return s_antBeta;
}
void
AntRoutingTable::AntBeta(double antBeta){
  s_antBeta = antBeta;
}

double
AntRoutingTable::PacketBeta(){
  return s_packetBeta;
}
void
AntRoutingTable::PacketBeta(double packetBeta){
  s_packetBeta = packetBeta;
}

double
AntRoutingTable::Gamma(){
  return s_gamma;
}
void
AntRoutingTable::Gamma(double gamma){
  s_gamma = gamma;
}

Time
AntRoutingTable::HopTime(){
  return s_hopTime;
}
void
AntRoutingTable::HopTime(Time hopTime){
  s_hopTime = hopTime;
}

double
AntRoutingTable::BestEstCoeff() {
  return s_bestEstCoeff;
}
void
AntRoutingTable::BestEstCoeff(double coeff) {
  s_bestEstCoeff = coeff;
}


} // namespace ant_routing
} // namespace ns3
