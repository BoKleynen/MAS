/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef PHEROMONE_ROUING_TABLE_ENTRY_H
#define PHEROMONE_ROUING_TABLE_ENTRY_H

#include <map>
#include "ns3/Ipv4Address.h"
#include ""

namespace ns3 {
namespace ant_routing {

using Pheromone = double;

//TODO do we only take the IP address to identify our neighbors? or do we
// include more information? ~> i would want seperation of concerns, keep track
// of neighbours somewhere else;

class PheromoneEntry {
public:
  /**
   * Queries the entry to get the route to the destination for a packet, this
   * chance is calculated based on the pheremone values and random chance
   */
  Ptr<Ipv4Route> GetRoute(Ipv4Address dest);

  /**
   * Updates the entry based on the information gathered by the ant
   * param nextHop: the neighbour the forward ant has chosen as a next hop
   * param travelTime: estimate of how long it took to travel from the current node
   *                   to the end node
   * param hops: the number of hops taken from this node to the destination node
   * //TODO check if routing table entries can dissapear between receival
   *        of the ant and table update
   */
  void UpdateEntry(Ipv4Address nextHop, Time travelTime, int32_t hops);

  /**
   * Return a vector containing all routes to the neighbours
   * that have not been visited yet by ants.
   */
  std::vector<Ipv4Routes> GetNoPheromoneRoutes();



  /**
   * Removes a neigbour from the routing table
   */
  void RemoveNeighbor(Ipv4Address addr);

  static void setHopTime(Time time);
  static void setGamma(double gamma);

private:
  std::map<Ipv4Address, Pheromone> m_pheromones;
  static Time HopTime;
  static double gamma;
  static Ptr<UniformRandomVariable> rand;
};


} // namespace ant_routing
} // namespace ns3

#endif // ANT_ROUTING_TABLE_ENTRY
