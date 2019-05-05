/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_H
#define ANT_H

#include "ns3/core-module.h"

namespace ns3 {
namespace ant_routing {

class AntHeader;
class AnthocNetRouting;

/**
 * Interface class that define the behavior of all ants
 */
class Ant : class Object {
public:
  /**
   * Type id used to hook the ant up to the metaprogramming system
   */
  static TypeId GetTypeId();

  virtual ~Ant();

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

  /**
   * Getter and setter for the header of the ant. This header is used
   * to guide the ant through the system.
   */
  Ptr<AntHeader> GetAntHeader();
  void SetAntHeader(Ptr<AntHeader> antHeader);
private:
  /**
   * Header of the ant
   */
  Ptr<AntHeader> m_header;
};

} // namespace ant_routing
} // namespace ns3

#endif
