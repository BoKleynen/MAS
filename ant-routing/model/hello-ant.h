#ifndef  HELLO_ANT_H
#define HELLO_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class HelloAnt : public Ant {
  HelloAnt() = default;
  HelloAnt(Ptr<Packet> packet);
  ~HelloAnt() = default;

  virtual void Visit(AnthocNetRouting router) override;

  static constexpr AntType species = AntType::HelloAnt;
};

using HelloQueen = AntQueenImpl<HelloAnt>;

}
}

#endif // HELLO_ANT_H
