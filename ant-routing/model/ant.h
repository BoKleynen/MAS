/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_H
#define ANT_H

#include "ns3/core-module.h"
#include "ns3/packet.h"
#include "ant-packet.h"
#include <memory>

namespace ns3 {
namespace ant_routing {

class AnthocnetRouting;
class Neighbor;

/**
 * Interface class that define the behavior of all ants
 */
class Ant {
public:

  Ant() = default;

  virtual ~Ant() = default;

  // Each ant has a specific role to perform when it visits the node
  // it will mutate some data or log certain entries.
  virtual void Visit(AnthocnetRouting router) = 0;

  void BroadcastPacket(Ptr<Packet> packet, AnthocnetRouting routing);
  void UnicastPacket(Ptr<Packet> packet, AnthocnetRouting routing, Neighbor neighbor);

  // Create a packet from the given ant
  virtual Ptr<Packet> ToPacket() = 0;
};

class AntQueen {
public:
  AntQueen() = default;
  ~AntQueen() = default;

  // queen creates a new ant from the given header
  virtual std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) = 0;

  virtual AntType GetAntType() = 0;
};

// Requirements for AntType: subclass of ant and constructor with a header
template<typename AntSpecies>
class AntQueenImpl : public AntQueen {
public:
  virtual AntType GetAntType() override {
    return AntSpecies::species;
  }

  virtual std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) override {
    // if the header number corresponds to the class
    if(!HasRightAntType(typeHeader)){
      return nullptr;
    }

    // TODO add some 'modify header' methods that does the mutations
    // automatically?
    // TODO maybe add some automatic verification of the package besides
    // having the right type. For example a forward and may not have a backward count
    // different from zero
    return std::make_shared<AntSpecies>(packet);
  }

  bool HasRightAntType(const AntTypeHeader& typeHeader) {
    return AntSpecies::species == typeHeader.GetAntType();
  }

  static constexpr AntType species = AntSpecies::Species;
};

} // namespace ant_routing
} // namespace ns3

#endif
