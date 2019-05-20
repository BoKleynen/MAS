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
  os << "PheromoneEntry { value: " << pe.Value() << "}";
  return os;
}


// Neighbor definition ---------------------------------------------------------

Neighbor::Neighbor() : Neighbor(Ipv4Address(), Ptr<NetDevice>()) { }

Neighbor::Neighbor(Ipv4Address addr) : Neighbor(addr, Ptr<NetDevice>()) { }

Neighbor::Neighbor(Ipv4Address addr, Ptr<NetDevice> device) : m_addr(addr), m_device(device) { }

Ptr<Ipv4Route>
Neighbor::CreateRoute(Ipv4Address source, Ipv4Address destination) {
  auto route = Create<Ipv4Route>();
  route->SetDestination(destination);
  route->SetSource(source);
  route->SetGateway(this->Address());
  route->SetOutputDevice(this->Device());

  return route;
}

Ipv4Address
Neighbor::Address() const {
  return m_addr;
}

void
Neighbor::Address(Ipv4Address addr) {
  m_addr = addr;
}

const Ptr<NetDevice>
Neighbor::Device() const {
  return m_device;
}

void
Neighbor::Device(Ptr<NetDevice> device) {
  m_device = device;
}

bool operator<(const Neighbor& lhs, const Neighbor& rhs) {
  return lhs.Address() < rhs.Address();
}

std::ostream& operator<<(std::ostream& os, const Neighbor& nb) {
  os << "Neighbor { address: " << nb.Address() << "}";
  return os;
}



// AntRoutingTable definition --------------------------------------------------
// static variables:
double AntRoutingTable::s_antBeta = 1.0;
double AntRoutingTable::s_packetBeta = 1.0;
double AntRoutingTable::s_gamma = 0.7;
Time   AntRoutingTable::s_hopTime = MilliSeconds(3);

// implementation of methods
AntRoutingTable::AntRoutingTable() { }

// methods related to generating routes
Ptr<Ipv4Route>
AntRoutingTable::RouteTo(const Ipv4Header& ipv4h) {
  return RouteTo(ipv4h.GetSource(), ipv4h.GetDestination(), PacketBeta());
}

Ptr<Ipv4Route>
AntRoutingTable::RouteTo(const AntHeader& ah){
  return RouteTo(ah.GetOrigin(), ah.GetDestination(), AntBeta());
}

Ptr<Ipv4Route>
AntRoutingTable::RouteTo(Ipv4Address source, Ipv4Address dest, double beta) {

  // case that no entries in the table (no neighbors) return empty pointer
  if (m_table.size() == 0) {
    return Ptr<Ipv4Route>();
  }

  double totalPheromone = TotalPheromone(dest, beta);
  double selectionPoint = GetRand();
  double accumulator = 0;
  auto neighbors = Neighbors();

  // loop over all entries except for the last one
  // TODO: check if neighbors.rbegin() == --neighbors.end()
  for(auto neighborIt = neighbors.begin(); neighborIt != --neighbors.end(); neighborIt++) {
    auto entryPtr = GetPheromone(neighborIt->Address(), dest);
    if (entryPtr != nullptr) {
      accumulator += (pow(entryPtr->Value(), beta) / totalPheromone);
    }
    if (selectionPoint <= accumulator) {
      return neighborIt->CreateRoute(source, dest);
    }
  }

  // return the last entry in case the loop completed.
  // This seperate case is needed to deal with rounding errors in the accumulator
  return (neighbors.rbegin())->CreateRoute(source, dest);
}

std::vector<Ptr<Ipv4Route>>
AntRoutingTable::NoPheromoneRoutes(const AntHeader& ah) {
  Ipv4Address source = ah.GetOrigin();
  Ipv4Address destination = ah.GetDestination();
  std::vector<Ptr<Ipv4Route>> routes;
  auto neighbors = Neighbors();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt ++) {
    auto entryPtr = GetPheromone(neighborIt -> Address(), destination);
    if (entryPtr == nullptr) {
      auto route = neighborIt->CreateRoute(source, destination);
      routes.push_back(route);
    }
  }

  return routes;
}

std::vector<Ptr<Ipv4Route>>
AntRoutingTable::BroadCastRouteTo(const AntHeader& ah) {
  Ipv4Address source = ah.GetOrigin();
  Ipv4Address destination = ah.GetDestination();
  std::vector<Ptr<Ipv4Route>> routes;
  auto neighbors = Neighbors();

  for(auto neighborIt = neighbors.begin(); neighborIt != neighbors.end(); neighborIt++) {
    auto route = neighborIt->CreateRoute(source, destination);
    routes.push_back(route);
  }

  return routes;
}



// methods related to pheromone management
void
AntRoutingTable::UpdatePheromoneEntry(Ipv4Address neighbor, Ipv4Address dest, Time timeEstimate,  uint32_t hops) {
  auto neighborTablePtr = GetNeighborTable(neighbor);
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
  pheromoneEntryPtr->HopCount(hops);
  pheromoneEntryPtr->TimeEstimate(timeEstimate);
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

const std::shared_ptr<PheromoneEntry>
AntRoutingTable::GetPheromone(Ipv4Address neighbor, Ipv4Address destination) {
  auto neighborTableIt = m_table.find(neighbor);

  if (neighborTableIt == m_table.end()) {
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
  auto neighborTableIt = m_table.find(neighbor);

  if (neighborTableIt == m_table.end()) {
    return;
  }

  // change the entry for the destination or create a new one for the
  // given destination in case there wasn't any.
  std::shared_ptr<NeighborTable> neighborTable = neighborTableIt->second;
  (*neighborTable)[destination] = std::make_shared<PheromoneEntry>(entry);
}

double AntRoutingTable::TotalPheromone(Ipv4Address dest, double beta) {
  double totalPheromone = 0;

  for(auto nbrTableIt = m_table.begin(); nbrTableIt != m_table.end(); nbrTableIt++) {
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
  m_table[nb] = std::make_shared<NeighborTable>();
}
void
AntRoutingTable::RemoveNeighbor(const Neighbor& nb) {
  m_table.erase(nb);
}

bool AntRoutingTable::IsNeighbor(Neighbor neighbor) {
  auto neighborTableIter = m_table.find(neighbor);
  return neighborTableIter != m_table.end();
}


// returns a vector containing all the neighbors registered in the
// routing table.
std::vector<Neighbor>
AntRoutingTable::Neighbors() {
  std::vector<Neighbor> neighbors;
  for (auto neighborTableIt = m_table.begin(); neighborTableIt != m_table.end(); neighborTableIt ++ ) {
    neighbors.push_back(neighborTableIt->first);
  }

  return neighbors;
}

std::shared_ptr<AntRoutingTable::NeighborTable> AntRoutingTable::GetNeighborTable(Neighbor neighbor) {
  auto neighborTableIt = m_table.find(neighbor);
  return neighborTableIt != m_table.end() ? neighborTableIt->second : std::make_shared<NeighborTable>();
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


} // namespace ant_routing
} // namespace ns3
