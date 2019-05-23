#ifndef ANT_QUEEN
#define ANT_QUEEN

namespace ns3 {
namespace ant_routing {

/**
 * Queen spawns based on the provided ant packets
 */
class AntQueen {
public:

  AntQueen();
  ~AntQueen();

  // TODO Ptr or shared_ptr?
  std::shared_ptr<Ant> SpawnFrom(const AntHeader& header);
  std::shared_ptr<Ant> SpawnNew(const std::string& antType);

  void AddAntType(const std::string& AntType, std::shared_ptr<Ant> prototypeAnt);

private:
  struct AntQueenImpl;
  std::shared_ptr<AntQueenImpl> m_impl;
};

}
}

#endif
