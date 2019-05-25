#ifndef  HELLO_ANT_H
#define HELLO_ANT_H

#include "ant.h"

namespace ns3 {
namespace ant_routing {

class HelloAnt : public Ant {
public:
  HelloAnt() = default;
  HelloAnt(Ptr<Packet> packet);
  ~HelloAnt() = default;

  virtual void Visit(AnthocnetRouting router) override;

  static constexpr AntType species = AntType::HelloAnt;
private:
  HelloHeader m_header;
};

using HelloQueen = AntQueenImpl<HelloAnt>;

} // namespace ant_routing
} // namespace ns3

#endif // HELLO_ANT_H
