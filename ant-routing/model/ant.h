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

  virtual uint32_t GetAntTypeId() = 0;
};

// Requirements for AntType: subclass of ant and constructor with a header
template<typename AntType>
class AntQueenImpl : public AntQueen {
public:
  virtual uint32_t GetAntTypeId() {
    return AntType::Type;
  }

  virtual std::shared_ptr<Ant> CreateFrom(const AntHeader& header) {
    // if the header number corresponds to the class
    if(AntType::antType == header.GetAntType()){

      // TODO add some 'modify header' methods that does the mutations
      // automatically?
      return std::make_shared<AntType>(header);
    }

    return nullptr;
  }
};

} // namespace ant_routing
} // namespace ns3

#endif
