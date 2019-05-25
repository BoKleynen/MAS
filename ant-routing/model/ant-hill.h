#ifndef ANT_HILL_H
#define ANT_HILL_H

#include <memory>
#include "ns3/packet.h"
#include "ant-packet.h"

namespace ns3 {
namespace ant_routing {

class AntQueen;
class Ant;

class AntHill {
public:
  AntHill();
  virtual ~AntHill();

  // creates an ant from the given header
  std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet);

  void AddQueen(std::shared_ptr<AntQueen> queen);

  std::shared_ptr<AntQueen> GetQueen(AntType antType);
private:
  struct AntHillImpl;
  std::shared_ptr<AntHillImpl> m_impl;
};

}
}

#endif // ANT_HILL
