#ifndef ANT_NEIGHBOR_H
#define ANT_NEIGHBOR_H

#include "ant-netdevice.h"

#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <memory>
#include <iostream>

namespace ns3 {
namespace ant_routing {

// TODO hook up the new AntNetDevice Class instead of NetDevice
// Class representing a neighbor node
struct Neighbor {
public:

  friend bool operator<(const Neighbor& lhs, const Neighbor& rhs);
  friend class AntRoutingTable;
  // todo add a default device as a static variable? allows to add based on address only
  Neighbor();
  Neighbor(Ipv4Address addr, AntNetDevice device);

  ~Neighbor();

  // creates a route fronm souce to destination based on the configuration
  // of the neighbor
  Ptr<Ipv4Route> CreateRoute(Ipv4Address source, Ipv4Address destination);

  Ipv4Address Address() const;
  void Address(Ipv4Address addr);

  AntNetDevice AntDevice();
  void AntDevice(AntNetDevice device);

  // submits a packet to the destination, placing the packet in the sending queue
  // param route:     Route containing next hop to destination
  // param packet:    the packet to deliver
  // param header:    Ipv4 header containing information about the packet for the network layer
  // param callback:  Callback function to be called once the packet is to be sumbitted
  void SumbitPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet, const Ipv4Header &header, UnicastCallback callback);

  // submits a packet to the destination, using the expedited lane. (used for ants for example)
  void SubmitExpeditedPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet, const Ipv4Header &header, UnicastCallback callback);

private:

  struct NeighborImpl;

  const std::shared_ptr<NeighborImpl> Data() const;
  std::shared_ptr<NeighborImpl> Data();

  void Data(std::shared_ptr<NeighborImpl> impl);
  // pointer that holds the data for the neighbor
  std::shared_ptr<NeighborImpl> m_impl;
};

// inequality operator, allows the DefaultNeighbor type to be a key of a
// std::map
bool operator<(const Neighbor& lhs, const Neighbor& rhs);


std::ostream& operator<<(std::ostream& os, const Neighbor& nb);

} // namespace ant_routing
} // namespace ns3

#endif
