#ifndef LINK_FAILURE_ANT_H
#define LINK_FAILURE_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class AlternativeRoute;

class LinkFailureAnt : public Ant {
public:
  LinkFailureAnt(Ptr<Packet> packet);
  LinkFailureAnt() = default;
  virtual ~LinkFailureAnt() = default;

  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  static constexpr AntType species = AntType::LinkFailureAnt;

  static bool PheromoneUpdatesOnFailureEnabled();
  static void PheromoneUpdatesOnFailureEnabled(bool val);

private:

  static bool s_pheromoneUpdatesOnFailure;

  LinkFailureNotification m_header;

  bool LoopDetection(Ipv4Address addr);
  void NextHop(AnthocnetRouting router, std::vector<AlternativeRoute> alternatives);
};

using LinkFailureQueen = AntQueenImpl<LinkFailureAnt>;

} // namespace ant_routing
} // namespace ns3

#endif
