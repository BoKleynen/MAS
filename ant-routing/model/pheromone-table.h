#ifndef PHEROMONE_TABLE_H
#define PHEROMONE_TABLE_H

#include <map>
#include <memory>
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ant-packet.h"

namespace ns3 {
namespace ant_routing {

class PheromoneTable {
public:

  // structure to contain the pheromone info
  struct PheromoneInfo {
  public:
    friend PheromoneTable;
    // default constructor
    PheromoneInfo();
    // value of the pheromone
    double Value() const;
    // the last hop count, used for repairing routes
    uint32_t HopCount() const;
    // the last time estimate, used for repairing routes
    Time TimeEstimate() const;
    // allows implicit conversion from pheromone info to their value
    operator double() const;

  private:
    double m_value;
    uint32_t m_hopCount;
    Time m_timeEstimate;
  };

  // returns a route providing the next hop to get to the destiantion for a packet
  Ptr<Ipv4Route> RouteTo(const Ipv4Header& ipv4h);

  // returns a route providing the next unicast hop to the destination for an ant
  Ptr<Ipv4Route> RouteTo(const AntHeader& ah);

  // updates the pheromone entries for the neighbor to the destination
  // param neighbor:    the next hop in the route
  // param dest:        the ultimate destination of the route
  // param traveltime:  the time estimate to reach the destination (provided by backwards ant)
  // param hops:        the number of hops the backwards ant has taken to reach the destination
  void UpdatePheromone(Ipv4Address neighbor, Ipv4Address dest, Time TravelTime, uint32_t hops);

  // getter for the pheromone information for a destination
  // at a given neighbor, in case there is no such entry, return the zero pheromone
  const std::shared_ptr<PheromoneInfo> GetPheromone(Ipv4Address neighbor, Ipv4Address dest);

  // Returns all routes to neighbors taht do not have a pheromone entry
  // for the destination provided in the ant header (ah)
  std::vector<Ptr<Ipv4Route>> GetNoPheromoneRoutes(const AntHeader& ah);

  std::vector<Ptr<Ipv4Route>> BroadCastRouteTo(const Ipv4Header& ipv4h);
  std::vector<Ptr<Ipv4Route>> BroadCastRouteTo(const AntHeader& ah);

  // methods for adding and removing neighbors
  void AddNeighbor(Ipv4Address addr);
  void RemoveNeighbor(Ipv4Address addr);
  // setters

  // setter for the output network device, we assume we're using wifi
  // so there is only one output device.
  void SetNetDevice(Ptr<NetDevice> device);

  // gamma is exponentation value for calculating new pheromone values
  static void SetGamma(double gamma);
  // exponent factor used to determine the explorative behavior of
  // unicast ants
  static void SetBetaAnt(double beta);
  // exponent factor used to determine the likelyness of using
  // alternative paths for unicast packets
  static void SetBetaPacket(double beta);
  // time estimate for how long making a hop wil take (static value)
  static void setHopTime(Time hopTime);


private:

  // generates random number between 0 and 1, usd for routing packets
  static double GetRand() {
    static Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable> ();
    return randGen -> GetValue(0, 1);
  }

  // generic function for creating next hop routes for a given destination
  Ptr<Ipv4Route> RouteTo(Ipv4Address source, Ipv4Address dest, double beta);

  // generic function for creating broadcast routes for both packets and ants
  std::vector<Ptr<Ipv4Route>> BroadCastRouteTo (Ipv4Address source, Ipv4Address dest);

  // sums all the pheromones in a weighted fashion for a given destionation,
  // used by the RouteTo function
  double GetWeightedTotalPheromoneFor(Ipv4Address dest, double beta);

  // creates route (from, to, via)
  Ptr<Ipv4Route> CreateRouteFor(Ipv4Address source, Ipv4Address dest, Ipv4Address neighbor);

  // fetch all the neighbors for the pheromone table
  std::vector<Ipv4Address> GetNeighborAddrs();

  // core of the pheromone table;
  using InnerTableType = std::map<Ipv4Address, std::shared_ptr<PheromoneInfo>>;
  std::map<Ipv4Address, std::shared_ptr<InnerTableType>> m_table;

  // device used for routing
  Ptr<NetDevice> m_device;

  // static variables used for routing
  static double s_gamma;
  static double s_betaAnt;
  static double s_betaPacket;
  static Time s_hopTime;
};

} // namespace ant_routing
} // namespace ns3

#endif
