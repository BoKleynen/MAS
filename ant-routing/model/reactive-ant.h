/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef REACTIVE_ANT_H
#define REACTIVE_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ReactiveAnt : public Ant {
  static TypeId GetTypeId();
};

}
}

#endif // REACTIVE_ANT_H
