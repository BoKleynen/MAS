#ifndef PROACTIVE_ANT_H
#define PROACTIVE_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ProactiveAnt : public Ant {
public:
  ProactiveAnt(Ptr<Packet> packet);

  ProactiveAnt() = default;
  virtual ~ProactiveAnt() = default;

  // Each ant has a specific role to perform when it visits the node
  // it will mutate some data or log certain entries
  virtual void Visit(AnthocnetRouting router) override;

  static constexpr AntType species = AntType::ProactiveForwardAnt;
};

using ProactiveQueen = AntQueenImpl<ProactiveAnt>;

} // namespace ant_routing
} // namespace ns3
#endif // PROACTIVE_ANT
