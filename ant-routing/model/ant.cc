/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant.h"
#include "ant-packet.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing{

Ant::~Ant(){
}

TypeId Ant::GetTypeId() {
  static TypeId tid = TypeId ("ns3::ant_routing::Ant")
    .SetParent<Object>()
    .SetGroupName("AntRouting");

  return tid;
}


Ptr<antHeader> GetAntheader() {
  return m_header;
}

void Ant::SetAntHeader(Ptr<AntHeader> header) {
  m_header = header;
}

} // namespace ant_routing
} // namespace ns3
