/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "reactive-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

TypeId ReactiveAnt::GetTypeId() {
  // TODO add configuration attributes here (allows us to configure the ants from a script)
  static TypeId tid = TypeId("ns3::ant_routing::ReactiveAnt")
    .SetParent<ForwardAnt>()
    .SetGroupName("AntRouting")
    .AddConstructor<ReactiveAnt>();

  return tid;
}

Ptr<ReactiveAnt> ReactiveAnt::ConstructPtr() const {
  return Create<ReactiveAnt>();
}


/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void ReactiveAnt::Visit(Ptr<AnthocnetRouting> router){
  //TODO implement
}

/**
 * Ant decides what to do next (may cause a broadcast, may launch
 * a backwards ant etc...).
 */
void ReactiveAnt::Route(Ptr<AnthocnetRouting> router){
 //TODO implement
}

} // namespace ant_routing
} // namespace ns3
