/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define REPAIR_ANT_H
#ifndef REPAIR_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {
class RepairAnt : public Ant {
  static TypeId GetTypeId();
};

} // namespace ant_routing
} // namespace ns3
#endif // REPAIR_ANT_H
