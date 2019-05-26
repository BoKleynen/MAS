#ifndef PROACTIVE_ANT_H
#define PROACTIVE_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ProactiveAnt : public Ant {
public:
  ProactiveAnt() = default;
  ProactiveAnt(Ipv4Address source, Ipv4Address destination);
  ProactiveAnt(Ptr<Packet> packet);
  virtual ~ProactiveAnt() = default;

  // Each ant has a specific role to perform when it visits the node
  // it will mutate some data or log certain entries
  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  static constexpr AntType species = AntType::ProactiveForwardAnt;
private:
  AntHeader m_header;
};

using ProactiveQueen = AntQueenImpl<ProactiveAnt>;

} // namespace ant_routing
} // namespace ns3
#endif // PROACTIVE_ANT
