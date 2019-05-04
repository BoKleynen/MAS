/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant.h"


TypeId Ant::GetTypeId() {
  static TypeId tid = TypeId ("ns3::ant_routing::ant")
    .setParent<Object>()
    .setGroupName("AntRouting")

  return tid;
}
