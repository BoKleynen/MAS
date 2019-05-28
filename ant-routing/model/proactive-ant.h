#ifndef PROACTIVE_ANT_H
#define PROACTIVE_ANT_H

#include "forward-ant.h"

namespace ns3 {
namespace ant_routing {

class ProactiveAnt : public ForwardAnt {
public:
  ProactiveAnt() = default;
  // create a proactive ant on demand
  ProactiveAnt(Ipv4Address source, Ipv4Address destination);
  // create a proactive ant from a previously created proactive ant
  ProactiveAnt(Ptr<Packet> packet);
  virtual ~ProactiveAnt() = default;

  // Each ant has a specific role to perform when it visits the node
  // it will mutate some data or log certain entries
  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  virtual AntType GetSpecies() {
    return species;
  }

  static double BroadcastProbability();
  static void   BroadcastProbability(double probability);

  static uint8_t MaxBroadcastCount();
  static void    MaxBroadcastCount(uint8_t count);

  static constexpr AntType species = AntType::ProactiveForwardAnt;
private:

  static double GetRand() {
    static Ptr<UniformRandomVariable> randGen = CreateObject<UniformRandomVariable> ();
    return randGen -> GetValue(0, 1);
  }

  static double   s_broadcastProbability;
  static uint8_t  s_maxBroadcastCount;

  // broadcasting at the beginning for explorative behavior
  bool HandleProbabilisticBroadcast(AnthocnetRouting router);
  // broadcasting at the end if no pheromone routes are found
  // kills the ant in case the broadcast count has run out
  bool HandleNoEntryBroadcast(AnthocnetRouting router);
};

using ProactiveQueen = AntQueenImpl<ProactiveAnt>;

} // namespace ant_routing
} // namespace ns3
#endif // PROACTIVE_ANT
