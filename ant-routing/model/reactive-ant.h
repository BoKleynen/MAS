#ifndef REACTIVE_ANT_H
#define REACTIVE_ANT_H
#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ReactiveAnt : public Ant {
public:

  ReactiveAnt(Ptr<Packet> packet);
  ReactiveAnt() = default;

  virtual ~ReactiveAnt() = default;
  virtual void Visit(AnthocnetRouting router) override;

  static constexpr AntType species = AntType::ReactiveForwardAnt;
};

template<>
class AntQueenImpl<ReactiveAnt> : public AntQueen {
public:
  virtual std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet>) override;
  virtual AntType GetAntType() override;

  bool CanBeAdmitted(const AntHeader& header);

  void UpdateGenerationData(const AntHeader& header);
  bool IsBetterAnt(const AntHeader& header);
  bool HasRightAntType(const AntTypeHeader& typeHeader);

  uint32_t m_generation;
  uint8_t m_bestHopCount;
  Time m_bestTime;

  // how much worse the parameters of the Ant may be when checked agianst
  // the best ant of the generation (broadcast);
  static double s_admissionRatio;
};

using ReactiveQueen = AntQueenImpl<ReactiveAnt>;


}
}

#endif // REACTIVE_ANT_H
