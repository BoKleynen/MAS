/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef PROACTIVE_ANT_H
#define PROACTIVE_ANT_H

#include "forward-ant.h"

namespace ns3 {
namespace ant_routing {

class AnthocNetRouting;


class ProactiveAnt : public ForwardAnt {
public:
  /**
   * Hook us up to the metaprogramming system
   */
  static TypeId GetTypeId();


  ProactiveAnt() = default;
  virtual ~ProactiveAnt() = default;

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
  virtual Ptr<ProactiveAnt> ConstructPtr() const override;
};

} // namespace ant_routing
} // namespace ns3
#endif // PROACTIVE_ANT
