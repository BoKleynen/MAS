/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "anthocnet-helper.h"

namespace ns3 {

  //TODO implement

  AnthocnetHelper* AnthocnetHelper::Copy (void) const {
    return nullptr;
  }

  Ptr<Ipv4RoutingProtocol> AnthocnetHelper::Create(Ptr<Node> node) const {
    return Ptr<Ipv4RoutingProtocol>();
  }
}
