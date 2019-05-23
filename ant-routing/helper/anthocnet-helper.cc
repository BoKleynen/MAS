/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "anthocnet-helper.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {

  //TODO implement

  AnthocnetHelper::AnthocnetHelper()
    : Ipv4RoutingHelper() {
    m_routerFactory.SetTypeId(ant_routing::AnthocnetRouting::TYPENAME);
  }

  AnthocnetHelper* AnthocnetHelper::Copy (void) const {
    return new AnthocnetHelper(*this);
  }

  Ptr<Ipv4RoutingProtocol> AnthocnetHelper::Create(Ptr<Node> node) const {
    auto router = m_routerFactory.Create<ant_routing::AnthocnetRouting> ();
    node -> AggregateObject(router);
    return router;
  }
}
