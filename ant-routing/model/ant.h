/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_H
#define ANT_H

#include "ns3/core-module.h"
#include "ant-packet.h"
#include <memory>

namespace ns3 {
namespace ant_routing {

class AnthocnetRouting;

/**
 * Interface class that define the behavior of all ants
 */
class Ant {
public:

  Ant() = default;
  Ant(const AntHeader& header);

  virtual ~Ant();

  // Each ant has a specific role to perform when it visits the node
  // it will mutate some data or log certain entries.
  virtual void Visit(AnthocnetRouting router) = 0;

  // getter and setter
  const AntHeader& Header();
  void Header(const AntHeader& antHeader);

private:
  /**
   * Header of the ant
   */
  AntHeader m_header;
};

class AntQueen {
public:
  AntQueen() = default;
  ~AntQueen() = default;

  // queen creates a new ant from the given header
  virtual std::shared_ptr<Ant> CreateFrom(const AntHeader& header) = 0;

  virtual AntType GetAntTypeId() = 0;
};

// Requirements for AntType: subclass of ant and constructor with a header
template<typename AntSpecies>
class AntQueenImpl : public AntQueen {
public:
  virtual AntType GetAntTypeId() override {
    return AntSpecies::antType;
  }

  virtual std::shared_ptr<Ant> CreateFrom(const AntHeader& header) override {
    // if the header number corresponds to the class
    if(!HasRightAntType(header)){
      return nullptr;
    }

    // TODO add some 'modify header' methods that does the mutations
    // automatically?
    // TODO maybe add some automatic verification of the package besides
    // having the right type. For example a forward and may not have a backward count
    // different from zero
    return std::make_shared<AntSpecies>(header);
  }

  bool HasRightAntType(const AntHeader& header) {
    return AntSpecies::antType == header.GetAntType();
  }
};

} // namespace ant_routing
} // namespace ns3

#endif
