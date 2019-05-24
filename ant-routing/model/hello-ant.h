#ifndef  HELLO_ANT_H
#define HELLO_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class HelloAnt : public Ant {
  HelloAnt() = default;
  HelloAnt(const AntHeader& header);
  ~HelloAnt() = default;

  virtual void Visit(const AntHeader& header) override;

  static constexpr AntType species = AntType::HelloAnt;
};

using HelloQueen = AntQueenImpl<HelloAnt>;

}
}

#endif // HELLO_ANT_H
