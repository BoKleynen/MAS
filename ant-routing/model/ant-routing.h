/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_ROUTING_H
#define ANT_ROUTING_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include <memory>

namespace ns3 {
namespace ant_routing {

// forward declarations
class AntRoutingTable;
class AntNetDevice;
class NeighborManager;
class AntHill;
class ReactiveQueue;

class AnthocnetRouting : public Ipv4RoutingProtocol {

public:

  friend class ReactiveQueue;
  friend class Ant;

  /**
   * port for exchanging anthocnet organisational messages
   */
  static constexpr uint32_t ANTHOCNET_PORT = 1012;
  static constexpr const char* TYPENAME = "ns3::ant_routing::AnthocnetRouting";

  /**
   * Type id function used to hook us up to the metaprogramming system
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor
   * Note: choose good default parameters, allow to change them later via
   * Set attribute (for example the ant-broadcast factor etc)
   */
  AnthocnetRouting();

  /**
   * default implementation of the destructor
   */
  virtual ~AnthocnetRouting();

 /**
  * Query the underlying routing table to get the Ipv4Route to the next hop
  * see src/internet/model/ipv4-route.h for more information on what we
  * have to provide.
  * Note: this message does simply perform a lookup, does not yet route the packet
  */
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) override;

  /**
   * Route ingress packets to the next destination, behavior of this method
   * is depending on the packet itself.
   * 1. The packet destination is the current node (idev) then call lcb
   * 2. The packet is unicast, forward to the next node by calling ucb
   * 3. call ecb in the remaining cases, we do not support multicast routing.
   *
   * Note: In our case we'll have to build routes if there are none available
   *       look at AODV for more inspiration
   */
  virtual bool RouteInput  (Ptr<const Packet> packet, const Ipv4Header &header,
                            Ptr<const NetDevice> ingressDevice, UnicastForwardCallback ufcb,
                            MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                            ErrorCallback ecb) override;

  /**
   * \param interface the index of the interface we are being notified about
   *
   * Protocols are expected to implement this method to be notified of the state change of
   * an interface in a node.
   */
  virtual void NotifyInterfaceUp (uint32_t interface) override;
  /**
   * \param interface the index of the interface we are being notified about
   *
   * Protocols are expected to implement this method to be notified of the state change of
   * an interface in a node.
   */
  virtual void NotifyInterfaceDown (uint32_t interface) override;

  /**
   * \param interface the index of the interface we are being notified about
   * \param address a new address being added to an interface
   *
   * Protocols are expected to implement this method to be notified whenever
   * a new address is added to an interface. Typically used to add a 'network route' on an
   * interface. Can be invoked on an up or down interface.
   */
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) override;

  /**
   * \param interface the index of the interface we are being notified about
   * \param address a new address being added to an interface
   *
   * Protocols are expected to implement this method to be notified whenever
   * a new address is removed from an interface. Typically used to remove the 'network route' of an
   * interface. Can be invoked on an up or down interface.
   */
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) override;

  /**
   * \param ipv4 the ipv4 object this routing protocol is being associated with
   *
   * Typically, invoked directly or indirectly from ns3::Ipv4::SetRoutingProtocol
   */
  virtual void SetIpv4 (Ptr<Ipv4> ipv4) override;

  /**
   * \brief Print the Routing Table entries
   *
   * \param stream The ostream the Routing table is printed to
   * \param unit The time unit to be used in the report
   */
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const override;

  // return the routing table used by the protocol
  AntRoutingTable GetRoutingTable();
  // return the neighbor manager, used to detect failure and such
  NeighborManager GetNeighborManager();
  // return the AntNetDevice, used to send data
  AntNetDevice GetDevice();
  // return the ant hill, allows to generate ants
  AntHill GetAntHill();
  // return the queue containing all the pending packets
  ReactiveQueue GetReactiveQueue();

  Ipv4Address GetAddress();
  Ipv4InterfaceAddress GetInterfaceAddress();

  static void SetHelloTimerInterval(Time interval);
  static Time GetHelloTimerInterval();

protected:
  virtual void DoInitialize() override;
private:

  // callback for when a message is received at the routing protocol socket
  void ReceiveAnt(Ptr<Socket> socket);

  // hello timer, will send hello messages to everyone in the vicinity
  void HelloTimerExpire();

  // install the sockets needed by the anthocnet routing
  void InstallSockets();
  // used to make the life of the neighbor monitor easier
  void InstallNeighborFactory();
  void InstallLinkFailureCallback();

  // broadcasts the packet to all the neighbor of the node in an expedited way
  void BroadcastExpedited(Ptr<Packet> packet);

  Ptr<Socket> GetUnicastSocket();

  // input routing helper methods, returning true if they could handle the
  // given input:

  // Handles the broadcasting by dropping all broadcasting packets
  // that are not aimed at Anthocnet (we don't know how to deal with that kind of traffic)
  bool HandleIngressBroadcasting(Ptr<const Packet> packet,
                            const Ipv4Header &header, uint32_t ingressInterfaceIndex,
                            LocalDeliverCallback lcb, ErrorCallback ecb);
  bool HandleIngressLocal(Ptr<const Packet> packet,
                            const Ipv4Header &header, uint32_t ingressInterfaceIndex,
                            LocalDeliverCallback lcb, ErrorCallback ecb);
  bool HandleIngressForward(Ptr<const Packet> packet,
                              const Ipv4Header& header, uint32_t ingressInterfaceIndex,
                              UnicastForwardCallback ufcb, ErrorCallback ecb);

  uint32_t GetInterfaceIndexForDevice(Ptr<const NetDevice> device);
  bool IsBroadCastForAnthocnet(Ptr<const Packet> packet, const Ipv4Header& header);

  // used as a random variable stream for some actions
  static double GetRand() {
    static Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable> ();
    return randGen -> GetValue(0, 1);
  }
  // statics
  static Time s_helloInterval;
  // constants:
  static constexpr const char* localhost = "127.0.0.1";

  // returns the first address attached to the interface
  // always returns the address at index 0 for the interface
  // this routing protocol now only accepts a single address attached to the interface.
  Ipv4Address GetAddressOf(Ptr<NetDevice> device);

  struct AnthocnetImpl;
  std::shared_ptr<AnthocnetImpl> m_impl;
};

} //namespace ant_routing
} //namespace ns3

#endif /* ANT_ROUTING_H */
