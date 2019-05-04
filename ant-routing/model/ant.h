/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_H
#define ANT_H

#include "ns3/core-model.h"

namespace ns3 {
namespace ant_routing {
/**
 * Interface class that define the behavior of all ants
 */
class Ant : class Object {

  /**
   * Type id used to hook the ant up to the metaprogramming system
   */
  static TypeId GetTypeId();

  Ant() = delete;
  virtual ~Ant();

  /**
   * All ants should be cloneable, this allows us to easilly construct
   * multiple ants from a single 'prototype ant'
   */
  virtual Ptr<Ant> Clone() = 0;

  /**
   * Each ant has a specific role to perform when it visits the node
   * it will mutate some data or log certain entries
   */
  virtual void Visit(Ptr<AnthocnetRouting> router) = 0;

  /**
   * Ant decides what to do next (may cause a broadcast, may launch
   * a backwards ant etc...).
   */
  virtual void Route(Ptr<AnthocnetRouting> router) = 0;

  // TODO get & set header
};

} // namespace ant_routing
} // namespace ns3

#endif
