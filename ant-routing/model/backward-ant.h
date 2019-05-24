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
  BackwardAnt(const AntHeader& header);
  BackwardAnt() = default;
  virtual ~BackwardAnt() = default;

  /**
   * Each ant has a specific role to perform when it visits the node
   * it will mutate some data or log certain entries
   */
  virtual void Visit(AnthocnetRouting router) override;

  static constexpr AntType antType = AntType::BackwardAnt;

};

using BackwardQueen = AntQueenImpl<BackwardAnt>;

} // namespace ant_routing
} // namespace ns3
#endif // BACKWARD_ANT_H
