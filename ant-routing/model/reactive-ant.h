/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef REACTIVE_ANT_H
#define REACTIVE_ANT_H

#include "forward-ant.h"

namespace ns3 {
namespace ant_routing {

class AnthocNetRouting;


class ReactiveAnt : public ForwardAnt {
public:
  /**
   * Hook us up to the metaprogramming system
   */
  static TypeId GetTypeId();


  ReactiveAnt() = default;

  virtual ~ReactiveAnt() = default;

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
  virtual Ptr<ReactiveAnt> ConstructPtr() const override;
private:
  // TODO flesh out the reactive ant
};

}
}

#endif // REACTIVE_ANT_H
