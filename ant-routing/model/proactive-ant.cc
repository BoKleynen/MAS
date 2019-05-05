/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "proactive-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

TypeId ProactiveAnt::TypeId() {
  static TypeId tid = TypeId("ns3::ant_routing::ProactiveAnt")
    .SetParent<ForwardAnt>()
    .SetGroupName("AntRouting")
    .AddConstructor<ProactiveAnt>();

  return tid;
}


Ptr<ProactiveAnt> ProactiveAnt::ConstructPtr() const {
  return Create<ProactiveAnt>();
}

/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void ProactiveAnt::Visit(Ptr<AnthocnetRouting> router) {

}

/**
 * Ant decides what to do next (may cause a broadcast, may launch
 * a backwards ant etc...).
 */
void ProactiveAnt::Route(Ptr<AnthocnetRouting> router) {

}

} // namespace ant_routing
} // namespace ns3
