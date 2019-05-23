/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "backward-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

BackwardAnt::BackwardAnt(const AntHeader& header)
  : Ant(header) {

}

/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void BackwardAnt::Visit(AnthocnetRouting router) {

}

} // namespace ant_routing
} // namespace ns3
