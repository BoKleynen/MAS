#ifndef REPAIR_ANT_H
#define REPAIR_ANT_H
#include "ant.h"

namespace ns3 {
namespace ant_routing {

class RepairAnt : public Ant {
public:

  RepairAnt() = default;
  RepairAnt(Ptr<Packet> packet);

  virtual ~RepairAnt() = default;
  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  static constexpr  AntType species = AntType::RouteRepairAnt;
};

using RepairQueen = AntQueenImpl<RepairAnt>;


} // namespace ns3
} // namespace ant_routing

#endif
