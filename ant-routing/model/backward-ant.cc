/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "backward-ant.h"
#include "ant-routing.h"


namespace ns3 {
namespace ant_routing {

BackwardAnt::BackwardAnt(const AntHeader& header) {
  NS_LOG_UNCOND("Backward ant created!");
}


BackwardAnt::BackwardAnt(Ptr<Packet> packet) {

}

/**
 * Each ant has a specific role to perform when it visits the node
 * it will mutate some data or log certain entries
 */
void BackwardAnt::Visit(AnthocnetRouting router) {

}

Ptr<Packet>
BackwardAnt::ToPacket() {
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
