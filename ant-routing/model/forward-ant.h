/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FORWARD_ANT_H
#define FORWARD_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class AnthocNetRouting;


class ForwardAnt : public Ant {
public:

  static TypeId getTypeId();

  ForwardAnt() = default;
  ~ForwardAnt() = default;

  /**
   * All forward ants should be cloneable, this allows us to easilly construct
   * multiple ants from a single 'prototype ant'. Note that the clone
   * operation creates shallow copy of the ant.
   */
  virtual Ptr<ForwardAnt> Clone() const;
protected:
  /**
   * Member function that constructs a new Ptr of the same type
   * used as template function for the Clone() operation.
   */
  virtual Ptr<ForwardAnt> ConstructPtr() const = 0;
};

}
}
#endif // FORWARD_ANT_H
