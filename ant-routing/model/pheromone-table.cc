#include "pheromone-table.h"
#include <cmath>

namespace ns3 {
namespace ant_routing {

// Pheromone table statics------------------------------------------------------
Time PheromoneTable::s_hopTime = MilliSeconds(3); // estimate for the default hop time
double PheromoneTable::s_betaPacket = 1; // beta for exponentation of packet routing
double PheromoneTable::s_betaAnt = 1; // beta for exponentiation of ant routing

// Pheromone definition---------------------------------------------------------
double PheromoneTable::PheromoneInfo::s_gamma = 0.7;
PheromoneTable::PheromoneInfo PheromoneTable::PheromoneInfo::defaultVal = PheromoneInfo();

PheromoneTable::PheromoneInfo::PheromoneInfo() : m_value(0) {}

double PheromoneTable::PheromoneInfo::Value() {
  return m_value;
}

void PheromoneTable::PheromoneInfo::Update(double extraPheromone) {
  m_value = s_gamma*m_value + (1-s_gamma)*extraPheromone;
}

// Pheromone table definition --------------------------------------------------
Ptr<Ipv4Route> PheromoneTable::PacketRouteTo(Ptr<Ipv4Header> ipv4h) {
  return RouteTo(ipv4h->GetSource(), ipv4h->GetDestination(), s_betaPacket);
}

/**
 * Updates the entry based on the information gathered by the ant
 * param destination: destination of the route
 * param nextHop: the neighbour the forward ant has chosen as a next hop
 * param travelTime: estimate of how long it took to travel from the current node
 *                   to the end node
 * param hops: the number of hops taken from this node to the destination node
 * //TODO check if routing table entries can dissapear between receival
 *        of the ant and table update
 * returns false in case there is no entry for the next hop... should have added neighbour
 */
bool PheromoneTable::UpdateEntry(Ipv4Address nextHop, Ipv4Address dest, Time travelTime, int32_t hops) {
  double extraPheromone = 0.5*(travelTime.GetSeconds() + hops*s_hopTime.GetSeconds());
  auto nbTableOpt = m_table.Lookup(nextHop);

  if(!nbTableOpt.ok()) {
    return false;
  }

  auto neighborTable = nbTableOpt.ref();
  auto EntryOpt = neighborTable.Lookup(dest);

  // case there isn't yet an entry. Do add one
  if(!EntryOpt.ok()) {
    auto Entry = neighborTable.Insert(dest, PheromoneInfo());
    Entry.Update(extraPheromone);
  }else {
    auto Entry = EntryOpt.ref();
    Entry.Update(extraPheromone);
  }

  return true;

}

/**
 * Returns all the routes to neighbours that do not have a pheromone entry
 * for a given destination.
 */
std::vector<Ptr<Ipv4Route>> PheromoneTable::GetNoPheromoneRoutes(Ipv4Address dest) {
  return std::vector<Ptr<Ipv4Route>> ();
}

std::vector<Ptr<Ipv4Route>> PheromoneTable::BroadcastAnt(Ptr<AntHeader> ah) {
  std::vector<Ipv4Address> nbAddrs = GetNeighborAddrs();
  std::vector<Ptr<Ipv4Route>> routes;
  for(auto iter  = nbAddrs.begin(); iter != nbAddrs.end(); iter++) {
    routes.push_back(CreateRouteFor(ah->GetOrigin(), ah->GetDestination(), *iter));
  }

  return routes;
}

/**
 * Returns the next hop for an ant in case unicast is desired
 * (ants may feel more adventurous)
 */
Ptr<Ipv4Route> PheromoneTable::AntRouteTo(Ptr<AntHeader> ah) {
  return RouteTo(ah->GetOrigin(), ah->GetDestination(), s_betaAnt);
}

/**
 * set the Ipv4 stack of the table where the protocol is associated with.
 */
void PheromoneTable::SetNetDevice(Ptr<NetDevice> device) {
  m_device = device;
}

void PheromoneTable::SetGamma(double gamma) {
  PheromoneInfo::s_gamma = gamma;
}

void PheromoneTable::SetHopTime(Time hopTime){
  s_hopTime = hopTime;
}

void PheromoneTable::SetBetaAnt(double betaAnt) {
  s_betaAnt = betaAnt;
}

void PheromoneTable::SetBetaPacket(double betaPacket) {
  s_betaPacket = betaPacket;
}

Ptr<Ipv4Route> PheromoneTable::RouteTo(Ipv4Address source, Ipv4Address dest, double beta) {
  // we first select a random value and then route the packet as soon as
  // the random value is smaller than the cumulative value
  double randVal = getRand();
  double totalPm = GetWeightedTotalPheromoneFor(dest, beta);
  double cumulativePmPercent = 0;
  // iterate until we reach the final neighbor
  for (auto nbIter = m_table.begin(); nbIter != m_table.end(); nbIter++) {
    auto neighbor = nbIter->first;
    double pm = GetPheromoneFor(nbIter->first, dest);
    cumulativePmPercent += pm/totalPm;

    if(cumulativePmPercent < randVal) {
      return CreateRouteFor(source, dest, neighbor);
    }
  }

  // failsafe case.
  return CreateRouteFor(source, dest, m_table.begin()-> first);
}

double PheromoneTable::GetWeightedTotalPheromoneFor(Ipv4Address dest, double beta) {
  double total = 0;

  for (auto iter = m_table.begin(); iter != m_table.end(); iter++) {
    auto neighborAddress = iter->first;
    double neighborPheromone = GetPheromoneFor(neighborAddress, dest);
    total = pow(neighborPheromone, beta);
  }

  return total;
}

std::vector<Ipv4Address> PheromoneTable::GetNeighborAddrs() {
  return m_table.GetKeys();
}



double PheromoneTable::GetPheromoneFor(Ipv4Address neighborAddr, Ipv4Address dest) {
  // force the default value in case there is no entry for either neigbour
  // or the destination. Will still give valid results since a default
  // table is empty and a default value for an absent pheromone is zero.
  auto neighbourTable = m_table.Lookup(neighborAddr).Value();
  return neighbourTable.Lookup(dest).Value();
}

Ptr<Ipv4Route> PheromoneTable::CreateRouteFor(Ipv4Address source, Ipv4Address dest, Ipv4Address neighbor) {
  auto route = Create<Ipv4Route>();
  route->SetDestination(dest);
  route->SetGateway(neighbor);
  route->SetSource(source);
  route->SetOutputDevice(m_device);

  return route;
}


} // namespace ant_routing
} // namespace ns3
