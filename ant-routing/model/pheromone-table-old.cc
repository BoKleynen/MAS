#include "pheromone-table-old.h"
#include <cmath>
#include "ns3/log.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE("Pheromone Table");


namespace ant_routing {

// Pheromone table statics------------------------------------------------------
double PheromoneTable::s_betaPacket = 1; // beta for exponentation of packet routing
double PheromoneTable::s_betaAnt = 1; // beta for exponentiation of ant routing

// Pheromone info definition---------------------------------------------------------
Time PheromoneTable::PheromoneInfo::s_hopTime = MilliSeconds(3); // estimate for the default hop time
double PheromoneTable::PheromoneInfo::s_gamma = 0.7;
PheromoneTable::PheromoneInfo PheromoneTable::PheromoneInfo::defaultVal = PheromoneInfo();

PheromoneTable::PheromoneInfo::PheromoneInfo() : m_value(0), m_lastHopCount(0), m_lastTimeEstimate(Time()) { }

double PheromoneTable::PheromoneInfo::Value() const {
  return m_value;
}

uint32_t PheromoneTable::PheromoneInfo::LastHopCount() const {
  return m_lastHopCount;
}

Time PheromoneTable::PheromoneInfo::LastTimeEstimate() const {
  return m_lastTimeEstimate;
}

void PheromoneTable::PheromoneInfo::Update(Time travelTime, uint32_t hops) {
  double extraPheromone = 0.5*(travelTime.GetSeconds() + hops*s_hopTime.GetSeconds());
  m_value = s_gamma*m_value + (1-s_gamma)*extraPheromone;
  NS_LOG_UNCOND("pheromone:" << m_value);
}

// Pheromone table definition --------------------------------------------------
Ptr<Ipv4Route> PheromoneTable::PacketRouteTo(Ptr<Ipv4Header> ipv4h) {
  return RouteTo(ipv4h->GetSource(), ipv4h->GetDestination(), s_betaPacket);
}

bool PheromoneTable::UpdateEntry( Ipv4Address neighbor, Ipv4Address dest, Time travelTime, uint32_t hops) {
  auto nbTableOpt = m_table.Lookup(neighbor);

  if(!nbTableOpt.ok()) {
    return false;
  }

  auto neighborTable = nbTableOpt.ref();
  auto entryOpt = neighborTable.Lookup(dest);

  // case there isn't yet an entry. Do add one
  if(!entryOpt.ok()) {
    auto nbTableOpt = m_table.Lookup(neighbor);
    auto neighborTable = nbTableOpt.ref();
    /*PheromoneInfo& entry = */ neighborTable.Insert(dest, PheromoneInfo());
    nbTableOpt = m_table.Lookup(neighbor);
    neighborTable = nbTableOpt.ref();
    PheromoneInfo& entry = neighborTable.Lookup(dest).ref();
    entry.Update(travelTime, hops);
    NS_LOG_UNCOND("EntryValue: " << entry.Value());

  }else {
    PheromoneInfo& entry = entryOpt.ref();
    entry.Update(travelTime, hops);
  }
  NS_LOG_UNCOND("New pheromone: " << static_cast<double>(GetPheromoneFor(neighbor, dest)));

  return true;

}


std::vector<Ptr<Ipv4Route>> PheromoneTable::GetNoPheromoneRoutes(const AntHeader& ah) {
  NS_LOG_UNCOND("NO ROUTE LOOKUP STARTED");
  std::vector<Ptr<Ipv4Route>> noPheromoneRoutes;
  for(auto nbrIter = m_table.begin(); nbrIter != m_table.end(); nbrIter ++) {
    auto optDest = nbrIter->second.Lookup(ah.GetDestination());
    NS_LOG_UNCOND("From addr: " << nbrIter->first);
    NS_LOG_UNCOND("To addr: " << ah.GetDestination());
    NS_LOG_UNCOND("Value: " << optDest.Value());
    NS_LOG_UNCOND("Refrence valid: " << optDest.ok());
    if (!optDest.ok()) {
      NS_LOG_UNCOND("no entry for: " << nbrIter->first << " -> " << ah.GetDestination());
      NS_LOG_UNCOND("entry value: " << GetPheromoneFor(ah.GetOrigin(), ah.GetDestination()).Value());
      noPheromoneRoutes.push_back(CreateRouteFor(ah.GetOrigin(), ah.GetDestination(), nbrIter->first));
    }
  }
  return noPheromoneRoutes;
}

std::vector<Ptr<Ipv4Route>> PheromoneTable::BroadcastAnt(const AntHeader& ah) {
  std::vector<Ipv4Address> nbAddrs = GetNeighborAddrs();
  std::vector<Ptr<Ipv4Route>> routes;
  for(auto iter  = nbAddrs.begin(); iter != nbAddrs.end(); iter++) {
    routes.push_back(CreateRouteFor(ah.GetOrigin(), ah.GetDestination(), *iter));
  }

  return routes;
}

/**
 * Returns the next hop for an ant in case unicast is desired
 * (ants may feel more adventurous)
 */
Ptr<Ipv4Route> PheromoneTable::AntRouteTo(const AntHeader& ah) {
  return RouteTo(ah.GetOrigin(), ah.GetDestination(), s_betaAnt);
}

/**
 * set the Ipv4 stack of the table where the protocol is associated with.
 */
void PheromoneTable::SetNetDevice(Ptr<NetDevice> device) {
  m_device = device;
}

void PheromoneTable::AddNeighbor(Ipv4Address addr) {

  auto nbrOpt = m_table.Lookup(addr);
  if (nbrOpt.ok()) {
    NS_LOG_INFO("Inserted a neighbor that did already exist");
  }

  m_table.Insert(addr, InnerTable());
}

void PheromoneTable::RemoveNeighbor(Ipv4Address addr) {
  auto nbrOpt = m_table.Lookup(addr);
  if(nbrOpt.ok()) {
    NS_LOG_INFO("Deleted a non existing element");
  }

  m_table.Delete(addr);
}


void PheromoneTable::SetGamma(double gamma) {
  PheromoneInfo::s_gamma = gamma;
}

void PheromoneTable::SetHopTime(Time hopTime){
  PheromoneInfo::s_hopTime = hopTime;
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



const PheromoneTable::PheromoneInfo& PheromoneTable::GetPheromoneFor(Ipv4Address neighborAddr, Ipv4Address dest) {
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
