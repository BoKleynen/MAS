#ifndef ANT_HILL_H
#define ANT_HILL_H

#include <memory>

namespace ns3 {
namespace ant_routing {

class AntHeader;
class AntQueen;
class Ant;

class AntHill {
public:
  AntHill();
  virtual ~AntHill();

  // creates an ant from the given header
  std::shared_ptr<Ant> CreateFrom(const AntHeader& header);

  void AddQueen(std::shared_ptr<AntQueen> queen);
private:
  struct AntHillImpl;
  std::shared_ptr<AntHillImpl> m_impl;
};

}
}

#endif // ANT_HILL
