#ifndef FORWARD_ANT_H
#define FORWARD_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ForwardAnt : public Ant {
public:
  ForwardAnt(AntHeader header);
  virtual ~ForwardAnt() = default;
  virtual Ptr<Packet> NextHopPacket(AnthocnetRouting router);
  virtual bool HandleUnicast(AnthocnetRouting router);
  virtual bool HandleAtDestination(AnthocnetRouting router);
  virtual AntType GetSpecies() = 0;
  virtual bool DetectLoop(Ipv4Address currentAddr);

protected:
  AntHeader m_header;
};

} // ant_routing
} // ns3

#endif // FORWARD_ANT_H
