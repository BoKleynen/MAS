/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef BACKWARD_ANT_H
#define BACKWARD_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class AnthocNetRouting;


/**
 * Class of Backward ants.
 */
class BackwardAnt : public Ant {
public:
  /**
   * Hook us up to the metaprogramming system
   */
  static TypeId GetTypeId();

  virtual BackwardAnt() = default;
  virtual ~BackwardAnt() = default;

  /**
   * Each ant has a specific role to perform when it visits the node
   * it will mutate some data or log certain entries
   */
  virtual void Visit(Ptr<AnthocnetRouting> router) override;

  /**
   * Ant decides what to do next (may cause a broadcast, may launch
   * a Backward ant etc...).
   */
  virtual void Route(Ptr<AnthocnetRouting> router) override;
};

} // namespace ant_routing
} // namespace ns3
#endif // BACKWARD_ANT_H
