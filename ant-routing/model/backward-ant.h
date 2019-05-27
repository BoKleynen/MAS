#ifndef BACKWARD_ANT_H
#define BACKWARD_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

/**
 * Class of Backward ants.
 */
class BackwardAnt : public Ant {
public:
  BackwardAnt(Ptr<Packet> packet);
  BackwardAnt(const AntHeader& header);
  BackwardAnt() = default;
  virtual ~BackwardAnt() = default;

  /**
   * Each ant has a specific role to perform when it visits the node
   * it will mutate some data or log certain entries
   */
  virtual void Visit(AnthocnetRouting router) override;
  virtual Ptr<Packet> ToPacket() override;

  static constexpr AntType species = AntType::BackwardAnt;

};

using BackwardQueen = AntQueenImpl<BackwardAnt>;

template<>
class AntQueenImpl<BackwardAnt> : public AntQueen {
public:

  static constexpr AntType species = BackwardAnt::species;

  virtual AntType GetAntType() override {
    return BackwardAnt::species;
  }

  virtual std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) override {
    // if the header number corresponds to the class
    if(!HasRightAntType(typeHeader)){
      return nullptr;
    }

    // TODO add some 'modify header' methods that does the mutations
    // automatically?
    // TODO maybe add some automatic verification of the package besides
    // having the right type. For example a forward and may not have a backward count
    // different from zero
    return std::make_shared<BackwardAnt>(packet);
  }

  bool HasRightAntType(const AntTypeHeader& typeHeader) {
    return BackwardAnt::species == typeHeader.GetAntType();
  }

  // uses the given header to construct a backward ant
  std::shared_ptr<BackwardAnt> CreateFromForwardAnt(const AntHeader& header) {
    // TODO: need for extra calls?
    return std::make_shared<BackwardAnt>(header);
  }
};

} // namespace ant_routing
} // namespace ns3
#endif // BACKWARD_ANT_H
