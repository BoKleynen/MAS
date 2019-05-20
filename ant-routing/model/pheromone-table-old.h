#ifndef PHEROMONE_TABLE_OLD_H
#define PHEROMONE_TABLE_OLD_H

//TODO make headers more fine grained
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ant-routing-table.h"
#include "ant-packet.h"

namespace ns3 {
namespace ant_routing {

class PheromoneTable {
public:

  struct PheromoneInfo{
    friend PheromoneTable;
    PheromoneInfo();
    void Update(Time travelTime, uint32_t hopCount);
    double Value() const;
    uint32_t LastHopCount() const;
    Time LastTimeEstimate() const;

    // allows for implicit typecasts
    operator double() const {
      return m_value;
    }

    static PheromoneInfo defaultVal;

  private:

    double m_value;
    uint32_t m_lastHopCount;
    Time m_lastTimeEstimate;

    static double s_gamma; // exponential smoothing
    static Time s_hopTime; // estimate for the default hop time
  };

  Ptr<Ipv4Route> PacketRouteTo(Ptr<Ipv4Header> dest);

  /**
   * Updates the entry based on the information gathered by the ant
   * param destination: destination of the route
   * param neighbor: the neighbour the forward ant has chosen as a next hop
   * param travelTime: estimate of how long it took to travel from the current node
   *                   to the end node
   * param hops: the number of hops taken from this node to the destination node
   * //TODO check if routing table entries can dissapear between receival
   *        of the ant and table update
   */
  bool UpdateEntry(Ipv4Address neighbor, Ipv4Address dest, Time travelTime, uint32_t hops);


  /**
   * Returns the next hop for an ant in case unicast is desired
   * (ants may feel more adventurous)
   */
  Ptr<Ipv4Route> AntRouteTo(const AntHeader& ah);


  /**
   * Returns all the routes to neighbours that do not have a pheromone entry
   * for a given destination.
   */
  std::vector<Ptr<Ipv4Route>> GetNoPheromoneRoutes(const AntHeader& ah);


  /**
   * contains all routes to known destinations
   */
  std::vector<Ptr<Ipv4Route>> BroadcastAnt(const AntHeader& ah);

  /**
   * Set the device on which the route must be output
   */
  void SetNetDevice(Ptr<NetDevice> device);

  /**
   * Adds a neigbor entry in the routing table
   * param addr: the address of the neighbour to add
   * In case there is already an entry for a neighbor this method
   * has no effect.
   */
  void AddNeighbor(Ipv4Address addr);

  /**
   * Removes the neighbor from the neighbour address
   * param addr: the addresses of the neighbour to be removed.
   * in case the neighbour does not exist, this method has no effect
   */
  void RemoveNeighbor(Ipv4Address addr);

  /**
   * setters for parameters used throughout the algorithm
   */
  // gamma: used to update the pheromone table
  static void SetGamma(double gamma);
  // hopTime: used to determine the pheromone value of a single
  // forward ant
  static void SetHopTime(Time hopTime);

  // beta value: used in calculating the route for an ant
  // regulates the explorativity of an ant
  static void SetBetaAnt(double betaAnt);

  // beta value: used in calculating the route for a packet
  // regulates the usage of alternative paths for a packet
  static void SetBetaPacket(double betaPacket);

  /**
   * Get the pheromone for a neigbor for a certain destination
   */
  const PheromoneInfo& GetPheromoneFor(Ipv4Address neighborAddr, Ipv4Address dest);

private:

  // random number generation for routing etc
  static double getRand() {
    static Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable> ();
    return randGen -> GetValue(0, 1);
  }

  /**
   * general calculation for the route to the next address
   * the beta determines the 'explorativity'
   */
  Ptr<Ipv4Route> RouteTo(Ipv4Address source, Ipv4Address dest, double beta);

  std::vector<Ipv4Address> GetNeighborAddrs();

  /**
   * Returns the weighted sum for the given destination for all
   * neighbours. Beta is used to exponentiate the pheromone value
   */
  double GetWeightedTotalPheromoneFor(Ipv4Address dest, double beta);

  /**
   * Creates an Ipv4Route to the given destination with as intermediary
   *
   */
  Ptr<Ipv4Route> CreateRouteFor(Ipv4Address source, Ipv4Address dest, Ipv4Address neighbor);

  // ip addressses, neighbor ip address, second arg the table
  // containing the pheromones for a certain destination
  using InnerTable = RoutingTable<PheromoneInfo>;
  RoutingTable<InnerTable> m_table;
  //TODO check if we can route the output to the same m_device
  // device to output all the routes on
  Ptr<NetDevice> m_device;

  static double s_betaPacket; // beta for exponentation of packet routing
  static double s_betaAnt; // beta for exponentiation of ant routing
};

} // namespace ant_routing
} // namespace ns3

#endif // PHEROMONE_TABLE_H
