#include "pheromone-table.h"
#include <cmath>

namespace ns3 {
namespace ant_routing {

// Pheromone table statics------------------------------------------------------
Time PheromoneTable::s_hopTime = MilliSeconds(3); // estimate for the default hop time
double PheromoneTable::s_betaPacket = 1; // beta for exponentation of packet routing
double PheromoneTable::s_betaAnt = 1; // beta for exponentiation of ant routing

// Pheromone definition---------------------------------------------------------
double PheromoneTable::Pheromone::s_gamma = 0.7;
PheromoneTable::Pheromone PheromoneTable::Pheromone::defaultVal = Pheromone();

PheromoneTable::Pheromone::Pheromone() : m_value(0) {}

double PheromoneTable::Pheromone::Value() {
  return m_value;
}

void PheromoneTable::Pheromone::Update(double extraPheromone) {
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
 */
void PheromoneTable::UpdateEntry(Ipv4Address dest, Ipv4Address nextHop, Time travelTime, int32_t hops) {

}

/**
 * Returns all the routes to neighbours that do not have a pheromone entry
 * for a given destination.
 */
std::vector<Ptr<Ipv4Route>> PheromoneTable::GetNoPheromoneRoutes(Ipv4Address dest) {
  return std::vector<Ptr<Ipv4Route>> ();
}

std::vector<Ptr<Ipv4Route>> PheromoneTable::Broadcast(Ipv4Address dest) {
  return std::vector<Ptr<Ipv4Route>> ();
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
  Pheromone::s_gamma = gamma;
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
  for (auto nbIter = m_table.cbegin(); nbIter != m_table.cend(); nbIter++) {
    auto neighbor = nbIter->first;
    double pm = GetPheromoneFor(nbIter->first, dest);
    cumulativePmPercent += pm/totalPm;

    if(cumulativePmPercent < randVal) {
      return CreateRouteFor(source, dest, neighbor);
    }
  }

  // failsafe case.
  return CreateRouteFor(source, dest, m_table.cbegin()-> first);
}

double PheromoneTable::GetWeightedTotalPheromoneFor(Ipv4Address dest, double beta) {
  double total = 0;

  for (auto iter = m_table.cbegin(); iter != m_table.cend(); iter++) {
    auto neighborAddress = iter->first;
    double neighborPheromone = GetPheromoneFor(neighborAddress, dest);
    total = pow(neighborPheromone, beta);
  }

  return total;
}


double PheromoneTable::GetPheromoneFor(Ipv4Address neighborAddr, Ipv4Address dest) {
  // force the default value in case there is no entry for either neigbour
  // or the destination. Will still give valid results since a default
  // table is empty and a default value for an absent pheromone is zero.
  auto neighbourTable = m_table.Lookup(neighborAddr).GetValue();
  return neighbourTable.Lookup(dest).GetValue();
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
