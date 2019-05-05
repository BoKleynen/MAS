/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_ROUTING_H
#define ANT_ROUTING_H

#include "ns3/core-module.h"
#include "ns3/ipv4-routing-protocol.h"

namespace ns3 {
namespace ant_routing {

class AnthocnetRouting : public Ipv4RoutingProtocol {

public:
  /**
   * port for exchanging anthocnet organisational messages
   */
  static const uint32_t ANTHOCNET_PORT;

  /**
   * Type id function used to hook us up to the metaprogramming system
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor
   * Note: choose good default parameters, allow to change them later via
   * Set attribute (for example the ant-broadcast factor etc)
   */
  AnthocnetRouting() = default;

  /**
   * default implementation of the destructor
   */
  virtual ~AnthocnetRouting() = default;

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
  virtual bool RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb) override;

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

};

} //namespace ant_routing
} //namespace ns3

#endif /* ANT_ROUTING_H */