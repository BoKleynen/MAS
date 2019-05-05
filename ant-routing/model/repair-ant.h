/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define REPAIR_ANT_H
#ifndef REPAIR_ANT_H

#include "forward-ant.h"

namespace ns3 {
namespace ant_routing {

class AnthocNetRouting;


class RepairAnt : public ForwardAnt {
public:
  static TypeId GetTypeId();

  RepairAnt() = default;
  virtual ~RepairAnt() = default;

  /**
   * Each ant has a specific role to perform when it visits the node
   * it will mutate some data or log certain entries
   */
  virtual void Visit(Ptr<AnthocnetRouting> router) override;

  /**
   * Ant decides what to do next (may cause a broadcast, may launch
   * a backwards ant etc...).
   */
  virtual void Route(Ptr<AnthocnetRouting> router) override;
protected:
  virtual Ptr<RepairAnt> ConstructPtr() const override;

};

} // namespace ant_routing
} // namespace ns3
#endif // REPAIR_ANT_H
