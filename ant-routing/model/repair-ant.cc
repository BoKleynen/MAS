/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "repair-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

TypeId RepairAnt::GetTypeId() {
  static TypeId tid = TypeId("ns3::ant_routing::RepairAnt")
    .SetParent<ForwardAnt>()
    .SetGroupName("AntRouting")
    .AddConstructor<RepairAnt>();

  return tid;
}

Ptr<RepairAnt> RepairAnt::ConstructPtr() {
  return Create<RepairAnt>
}


/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void RepairAnt::Visit(Ptr<AnthocnetRouting> router){

}

/**
 * Ant decides what to do next (may cause a broadcast, may launch
 * a backwards ant etc...).
 */
void RepairAnt::Route(Ptr<AnthocnetRouting> router){

}

} // namespace ant_routing
} // namespace ns3
