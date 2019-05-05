/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "backward-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

TypeId BackwardAnt::GetTypeId() {
  // TODO add configuration attributes here (allows us to configure the ants from a script)
  static TypeId tid = TypeId("ns3::ant_routing::BackardsAnt")
    .SetParent<ForwardAnt>()
    .SetGroupName("AntRouting");

    return tid;
}

/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void BackwardAnt::Visit(Ptr<AnthocnetRouting> router) {

}

/**
 * Ant decides what to do next (may cause a broadcast, may launch
 * a Backward ant etc...).
 */
void BackwardAnt::Route(Ptr<AnthocnetRouting> router) {

}

} // namespace ant_routing
} // namespace ns3
