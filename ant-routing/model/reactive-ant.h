#ifndef REACTIVE_ANT_H
#define REACTIVE_ANT_H
#include "ant.h"

namespace ns3 {
namespace ant_routing {

class ReactiveAnt : public Ant {
public:

  ReactiveAnt(const AntHeader& header);
  ReactiveAnt() = default;

  virtual ~ReactiveAnt() = default;
  virtual void Visit(AnthocnetRouting router) override;

  static constexpr AntType antType = AntType::ReactiveForwardAnt;
};

}
}

#endif // REACTIVE_ANT_H
