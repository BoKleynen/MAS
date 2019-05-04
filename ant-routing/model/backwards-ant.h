/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef BACKWARDS_ANT_H
#define BACKWARDS_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class BackwardsAnt : public Ant {
  static TypeId GetTypeId();
};

} // namespace ant_routing
} // namespace ns3
#endif // BACKWARDS_ANT_H
