#include "pheromone-table.h"

namespace ns3 {
namespace ant_routing {

// static variable definition --------------------------------------------------
double PheromoneTable::s_gamma = 0.7;
double PheromoneTable::s_betaAnt = 1;
double PheromoneTable::s_betaPacket = 1;
Time PheromoneTable::s_hopTime = MilliSeconds(3);

// PheromoneInfo definition ----------------------------------------------------
// default constructor
PheromoneTable::PheromoneInfo::PheromoneInfo() : m_value(0), m_hopCount(0), m_timeEstimate(Seconds(0)) { }
// value of the pheromone
double PheromoneTable::PheromoneInfo::Value() const {
  return m_value;
}
// the last hop count, used for repairing routes
uint32_t PheromoneTable::PheromoneInfo::HopCount() const {
  return m_hopCount;
}
// the last time estimate, used for repairing routes
Time PheromoneTable::PheromoneInfo::TimeEstimate() const {
  return m_timeEstimate;
}
// allows implicit conversion from pheromone info to their value
PheromoneTable::PheromoneInfo::operator double() const {
  return Value();
}

// PheromoneTable definition ---------------------------------------------------

// generic function for creating next hop routes for a given destination
Ptr<Ipv4Route> PheromoneTable::RouteTo(Ipv4Address source, Ipv4Address dest, double beta) {
  // we first select a random value and then route the packet as soon as
  // the random value is smaller than the cumulative value
  double randVal = GetRand();
  double totalPm = GetWeightedTotalPheromoneFor(dest, beta);
  double cumulativePmPercent = 0;
  // iterate until we reach the final neighbor
  for (auto nbIter = m_table.begin(); nbIter != m_table.end(); nbIter++) {
    auto neighbor = nbIter->first;
    double pm = *GetPheromone(neighbor, dest);
    cumulativePmPercent += pm/totalPm;

    if(cumulativePmPercent < randVal) {
      return CreateRouteFor(source, dest, neighbor);
    }
  }

  // failsafe case.
  return CreateRouteFor(source, dest, m_table.begin()-> first);
}

// sums all the pheromones in a weighted fashion for a given destionation,
// used by the RouteTo function
double PheromoneTable::GetWeightedTotalPheromoneFor(Ipv4Address dest, double beta){
  double totalPheromone = 0;
  for (auto iter = m_table.begin(); iter != m_table.end(); iter++) {
    auto neighborAddress = iter->first;
    double neighborPheromone = *GetPheromone(neighborAddress, dest);
    totalPheromone = pow(neighborPheromone, beta);
  }

  return totalPheromone;
}

// creates route (from, to, via)
Ptr<Ipv4Route> PheromoneTable::CreateRouteFor(Ipv4Address source, Ipv4Address dest, Ipv4Address neighbor){
  auto route = Create<Ipv4Route>();
  route->SetDestination(dest);
  route->SetGateway(neighbor);
  route->SetSource(source);
  route->SetOutputDevice(m_device);

  return route;
}

// fetch all the neighbors for the pheromone table
std::vector<Ipv4Address> PheromoneTable::GetNeighborAddrs(){
  std::vector<Ipv4Address> neighbors;

  for(auto neighborIter = m_table.begin(); neighborIter != m_table.end(); neighborIter++) {
    neighbors.push_back(neighborIter->first);
  }

  return neighbors;
}

// returns a route providing the next hop to get to the destiantion for a packet
Ptr<Ipv4Route> PheromoneTable::RouteTo(const Ipv4Header& ipv4h) {
  return RouteTo(ipv4h.GetSource(), ipv4h.GetDestination(), s_betaPacket);
}

// returns a route providing the next unicast hop to the destination for an ant
Ptr<Ipv4Route>
PheromoneTable::RouteTo(const AntHeader& ah) {
  return RouteTo(ah.GetOrigin(), ah.GetDestination(), s_betaAnt);
}

// updates the pheromone entries for the neighbor to the destination
// param neighbor:    the next hop in the route
// param dest:        the ultimate destination of the route
// param traveltime:  the time estimate to reach the destination (provided by backwards ant)
// param hops:        the number of hops the backwards ant has taken to reach the destination
void PheromoneTable::UpdatePheromone(Ipv4Address neighbor, Ipv4Address dest, Time TravelTime, uint32_t hops){

}

// getter for the pheromone information for a destination
// at a given neighbor, in case there is no such entry, return the zero pheromone
const std::shared_ptr<PheromoneTable::PheromoneInfo>
PheromoneTable::GetPheromone(Ipv4Address neighbor, Ipv4Address dest){

  auto neighborIt = m_table.find(neighbor);

  if(neighborIt == m_table.end()) {
    return std::make_shared<PheromoneInfo>();
  }

  auto destIt = neighborIt->second->find(dest);

  if(destIt == neighborIt->second->end()) {
    return std::make_shared<PheromoneInfo>();
  }

  return *destIt;
}

// Returns all routes to neighbors taht do not have a pheromone entry
// for the destination provided in the ant header (ah)
std::vector<Ptr<Ipv4Route>>
PheromoneTable::GetNoPheromoneRoutes(const AntHeader& ah){

}

std::vector<Ptr<Ipv4Route>>
PheromoneTable::BroadCastRouteTo(const Ipv4Header& ipv4h) {
  return BroadCastRouteTo(ipv4h.GetSource(), ipv4h.GetDestination());
}

std::vector<Ptr<Ipv4Route>>
PheromoneTable::BroadCastRouteTo(const AntHeader& ah) {
  return BroadCastRouteTo(ah.GetOrigin(), ah.GetDestination());
}

// generic function for creating broadcast routes for both packets and ants
std::vector<Ptr<Ipv4Route>>
PheromoneTable::BroadCastRouteTo(Ipv4Address source, Ipv4Address dest) {
  std::vector<Ipv4Address> nbAddrs = GetNeighborAddrs();
  std::vector<Ptr<Ipv4Route>> routes;
  for(auto iter  = nbAddrs.begin(); iter != nbAddrs.end(); iter++) {
    routes.push_back(CreateRouteFor(source, destination, *iter));
  }

  return routes;
}

// methods for adding and removing neighbors
void
PheromoneTable::AddNeighbor(Ipv4Address addr) {
  m_table[addr] = std::make_shared<InnerTableType>();
}
void
PheromoneTable::RemoveNeighbor(Ipv4Address addr) {
  m_table.erase(addr);
}
// setters

// setter for the output network device, we assume we're using wifi
// so there is only one output device.
void
PheromoneTable::SetNetDevice(Ptr<NetDevice> device) {
  m_device = device;
}

// gamma is exponentation value for calculating new pheromone values
void
PheromoneTable::SetGamma(double gamma) {
  s_gamma = gamma;
}
// exponent factor used to determine the explorative behavior of
// unicast ants
void
PheromoneTable::SetBetaAnt(double beta){
  s_betaAnt = beta;
}
// exponent factor used to determine the likelyness of using
// alternative paths for unicast packets
void
PheromoneTable::SetBetaPacket(double beta){
  s_betaPacket = beta;
}
// time estimate for how long making a hop wil take (static value)
void
PheromoneTable::setHopTime(Time hopTime){
  s_hopTime = hopTime;
}


} // namespace ns3
} // namespace ant_routing
