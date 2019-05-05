#ifndef PHEROMONE_TABLE_H
#define PHEROMONE_TABLE_H

//TODO make headers more fine grained
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

namespace ns3 {
namespace ant_routing {

class PheromoneTable {
public:

  Ptr<Ipv4Route> RouteTo(Ipv4Address dest);

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
  void UpdateEntry(Ipv4Address dest, Ipv4Address nextHop, Time travelTime, int32_t hops);


  /**
   * Returns the next hop for an ant in case unicast is desired
   * (ants may feel more adventurous)
   */
  Ptr<Ipv4Route> AntRouteTo(Ipv4Address dest);


  /**
   * Returns all the routes to neighbours that do not have a pheromone entry
   * for a given destination.
   */
  std::vector<Ptr<Ipv4Route>> GetNoPheromoneRoutes(Ipv4Address dest);


  /**
   * contains all routes to known destinations
   */
  std::vector<Ipv4Routes> Broadcast(Ipv4Address dest){

  }

  /**
   * Set the device on which the route must be output
   */
  void setNetDevice(Ptr<NetDevice> device);

  /**
   * setters for parameters used throughout the algorithm
   */
  // gamma: used to update the pheromone table
  static void SetGamma(double gamma);
  // hopTime: used to determine the pheromone value of a single
  // forward ant
  static void SetHopTime(double hopTime);

  // beta value: used in calculating the route for an ant
  // regulates the explorativity of an ant
  static void SetBetaAnt(double betaAnt);

  // beta value: used in calculating the route for a packet
  // regulates the usage of alternative paths for a packet
  static void SetBetaPacket(double betaPacket);
private:

  struct Pheromone{
    void update(double extraPheromone);
    double value();
    double m_value;
    static double s_gamma;
    static Pheromone defaultVal;
  }

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

  /**
   * Returns the weighted sum for the given destination for all
   * neighbours. Beta is used to exponentiate the pheromone value
   */
  double GetWeigthedTotalPheromoneFor(Ipv4Address dest, double beta);

  /**
   * Get the pheromone for a neigbor for a certain destination
   */
  double GetPheromoneFor(Ipv4Address neighborAddr; Ipv4Address dest);

  /**
   * Creates an Ipv4Route to the given destination with as intermediary
   *
   */
  Ptr<Ipv4Route> CreateRouteFor(Ipv4Address source, Ipv4Address dest, Ipv4Address neighbor);

  // ip addressses, neighbor ip address, second arg the table
  // containing the pheromones for a certain destination
  RoutingTable<RoutingTable<Pheromone>> m_table;
  //TODO check if we can route the output to the same m_device
  // device to output all the routes on
  Ptr<NetDevice> m_device;

  static Time s_hopTime; // estimate for the default hop time
  static double s_betaPacket; // beta for exponentation of packet routing
  static double s_betaAnt; // beta for exponentiation of ant routing
};

} // namespace ant_routing
} // namespace ns3

#endif // PHEROMONE_TABLE_H
