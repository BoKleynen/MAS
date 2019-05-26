#ifndef REACTIVE_ANT_H
#define REACTIVE_ANT_H
#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ReactiveAnt : public Ant {
public:
  ReactiveAnt() = default;
  ReactiveAnt(Ipv4Address source, Ipv4Address destination, uint32_t generation);
  ReactiveAnt(Ptr<Packet> packet);
  virtual ~ReactiveAnt() = default;

  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  static constexpr AntType species = AntType::ReactiveForwardAnt;
private:
  AntHeader m_header;
};

template<>
class AntQueenImpl<ReactiveAnt> : public AntQueen {
public:
  AntQueenImpl();
  ~AntQueenImpl();
  // creates a new forward ant
  // param source: the source of the ant (sender of the ant)
  // param destination: the destiantion of the ant
  // note that the ant is given the right generational number to allow
  // for duplicate filtering. (new reactive ants should thus only be generated)
  // via the reactive queen of the routing algorithm
  std::shared_ptr<Ant> CreateNew(Ipv4Address source, Ipv4Address destination);
  // creates reactive ants based on the given packet. This function also performs
  // duplicate filtering and validity checking, returning a null pointer in case of defects.
  virtual std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) override;
  virtual AntType GetAntType() override;

  static constexpr AntType species = ReactiveAnt::species;

private:
  struct GenerationInfo;
  struct ReactiveQueenImpl;

  // statics:
  static bool HasRightAntType(const AntTypeHeader& typeHeader);
  // how much worse the parameters of the Ant may be when checked agianst
  // the best ant of the generation (broadcast);
  static double s_admissionRatio;

  // dynamics:
  std::shared_ptr<GenerationInfo> getGenerationInfo();
  std::shared_ptr<ReactiveQueenImpl> m_impl;
};

using ReactiveQueen = AntQueenImpl<ReactiveAnt>;


}
}

#endif // REACTIVE_ANT_H
