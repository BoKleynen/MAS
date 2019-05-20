#ifndef ANT_ROUTING_TABLE_H
#define ANT_ROUTING_TABLE_H

#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ant-packet.h"

#include <memory>
#include <map>

namespace ns3 {
namespace ant_routing {

class PheromoneEntry;
class Neighbor;
class AntRoutingTable;

struct PheromoneEntry {
public:
  friend AntRoutingTable;
  PheromoneEntry();
  PheromoneEntry(double value, uint32_t hopCount, Time timeEstimate);

  double Value() const;
  void Value(double value);

  uint32_t HopCount() const;
  void HopCount(uint32_t hopCount);

  Time TimeEstimate() const;
  void TimeEstimate(Time timeEstimate);

private:
  double m_value;
  uint32_t m_hopCount;
  Time m_timeEstimate;
};

std::ostream& operator<<(std::ostream& os, const PheromoneEntry& pe);


// Impelentation of a simple neighbor node.
struct Neighbor {
public:

  friend bool operator<(const Neighbor& lhs, const Neighbor& rhs);

  // todo add a default device as a static variable? allows to add based on address only
  Neighbor();
  Neighbor(Ipv4Address addr); // constructor for implicit conversion from ip address to neighbours
  Neighbor(Ipv4Address addr, Ptr<NetDevice> device);

  // creates a route fronm souce to destination based on the configuration
  // of the neighbor
  Ptr<Ipv4Route> CreateRoute(Ipv4Address source, Ipv4Address destination);

  Ipv4Address Address() const;
  void Address(Ipv4Address addr);

  const Ptr<NetDevice> Device() const;
  void Device(Ptr<NetDevice> device);

private:
  // the ip address of the neighbor
  Ipv4Address m_addr;
  // the device on which to output data destined for the neighbor
  Ptr<NetDevice> m_device;
};

// inequality operator, allows the DefaultNeighbor type to be a key of a
// std::map
bool operator<(const Neighbor& lhs, const Neighbor& rhs);


std::ostream& operator<<(std::ostream& os, const Neighbor& nb);

/**
 * Class representing the routing table used to route ants and packages.
 * For each destination in the table, a pheromone value is kept for each of the
 * neighbors of the current node which allows for stochasitical routing.
 */
class AntRoutingTable {
public:
  // todo: check if default constructors are good enough, or modification is needed.
  AntRoutingTable();

  // Routing methods:

  // Creats an IPV4Route based on the supplied header.
  // If the routing table has no neighbors returns a nullpointer
  // In case that there is no pheromone for any of the neighbors, returns
  // an arbitrary route starting in one of the neighbors.
  Ptr<Ipv4Route> RouteTo(const Ipv4Header& ipv4h);
  Ptr<Ipv4Route> RouteTo(const AntHeader& ah);

  // Creates routes to the destinaton with as next hop all the neighbor nodes
  // That do not yet have any pheromone value.
  std::vector<Ptr<Ipv4Route>> NoPheromoneRoutes(const AntHeader& ah);

  // Creates routes to the destination with as next hop each of the neighbor nodes
  std::vector<Ptr<Ipv4Route>> BroadCastRouteTo(const AntHeader& ah);

  // Phermone related methods:

  // updates the pheromone entris for the neighbor to the destination. In case
  // that the neighbor doesn't already have an entry for the destination creates one
  // in case that the the 'neighbor' address is not the address of a neighbor silently
  // ignores the update
  // param neighbor:    the next hop in the route
  // param dest:        the ultimate destination of the route
  // param traveltime:  the time estimate to reach the destination (provided by backwards ant)
  // param hops:        the number of hops the backwards ant has taken to reach the destination
  void UpdatePheromoneEntry(Ipv4Address neighbor, Ipv4Address dest, Time TravelTime, uint32_t hops);

  // checks if there are any pheromone entries for the given destination
  bool HasPheromoneEntryFor(Ipv4Address destination);
  // checks if the provided neigbor has an entry in its pheromone table for the
  // given destination.
  bool HasPheromoneEntryFor(Ipv4Address neighbor, Ipv4Address destination);
  // gets the pheromone entry for the given neighbor and destination address
  const std::shared_ptr<PheromoneEntry> GetPheromone(Ipv4Address neighbor, Ipv4Address destination);
  // setter for pheromone values.
  void SetPheromoneAt(Ipv4Address neighbor, Ipv4Address destination, const PheromoneEntry& entry);

  // neighbor management:
  void AddNeighbor(const Neighbor& nb);
  void RemoveNeighbor(const Neighbor& nb);

  // checks if the given address is the address of a neighbor node
  bool IsNeighbor(Neighbor neighbor);
  // returns a vector containing all the neighbors registered in the
  // routing table.
  std::vector<Neighbor> Neighbors();

  // static variables to configure the calulcations on the ant-routing table.
  static double AntBeta();
  static void   AntBeta(double antBeta);

  static double PacketBeta();
  static void   PacketBeta(double packetBeta);

  static double Gamma();
  static void   Gamma(double gamma);

  static Time   HopTime();
  static void   HopTime(Time hopTime);

private:

  // instance variables:
  using DestAddr      = Ipv4Address;
  using NeighborTable = std::map<DestAddr, std::shared_ptr<PheromoneEntry>>;
  std::map<Neighbor, std::shared_ptr<NeighborTable>> m_table;

  // static variables:
  static double s_antBeta; // exploration exponent for the ants
  static double s_packetBeta; // exploration exponent for the packets
  static double s_gamma; // influence factor for new pheromones
  static Time   s_hopTime; // time to make a hop in the system (estimate)

  // auxillary methods

  // general function that generates a route from source to destination based
  // on the data in the table and the provided bete (explorative behavior)
  Ptr<Ipv4Route> RouteTo(Ipv4Address source, Ipv4Address destination, double beta);

  // retrieves the pheromone table of a single neighbor. In case there is no
  // such entry, return a nullpointer
  std::shared_ptr<NeighborTable> GetNeighborTable(Neighbor neighbor);

  // calculates the total pheromone for a destination with a given
  // beta which serves to configure the explorative behavior of the packet.
  double TotalPheromone(Ipv4Address dest, double beta);

  // generates random number between 0 and 1, usd for routing packets
  static double GetRand() {
    static Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable> ();
    return randGen -> GetValue(0, 1);
  }
};

} // ant_routing
} // ns3

#endif // ANT_ROUTING_TABLE_H
