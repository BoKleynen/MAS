/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANTHOCNET_HELPER_H
#define ANTHOCNET_HELPER_H

#include "ns3/ant-routing.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"

// helpers are always located in the ns3 namespace
namespace ns3 {

// TODO is still barebones, we may want to add some fluff to ease the setup
class AnthocnetHelper : public Ipv4RoutingHelper {

public:

  /**
   * Default constructor of the helper
   * // TODO instantiate the real helper, default is such that it compiles
   */
  AnthocnetHelper();

  /**
   * use default implementation of destructor (no fancy stuff on deletion)
   */
  virtual ~AnthocnetHelper () = default;

  /**
   * Copy constructor returning a pointer
   */
  virtual AnthocnetHelper* Copy (void) const override;

  /**
   * create a new routing protocol and install it on the node
   */
  virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

private:
  /**
   * The factory used to create an Anthocnet routing object
   */
  ObjectFactory m_routerFactory;

};

} //namespace ns

#endif /* ANTHOCNET_HELPER_H */
