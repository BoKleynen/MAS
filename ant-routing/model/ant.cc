/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing{

Ant::Ant(const AntHeader& header)
  : m_header(header) { }

Ant::~Ant(){
}

const AntHeader& Ant::Header() {
  return m_header;
}

void Ant::Header(const AntHeader& header) {
  m_header = header;
}

} // namespace ant_routing
} // namespace ns3
