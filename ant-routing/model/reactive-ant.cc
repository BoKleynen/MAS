#include "reactive-ant.h"
#include "ant-routing.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ReactiveAnt");

namespace ant_routing {

ReactiveAnt::ReactiveAnt(Ipv4Address source, Ipv4Address destination, uint32_t generation)
  : m_header(AntHeader()){
  m_header.SetSource(source);
  m_header.SetDestination(destination);
  m_header.SetGeneration(generation);
  m_header.AddVisitedNode(source);
}

ReactiveAnt::ReactiveAnt(Ptr<Packet> packet) {

}

void ReactiveAnt::Visit(AnthocnetRouting router){
  //TODO implement
}

Ptr<Packet>
ReactiveAnt::ToPacket() {
  return Ptr<Packet>();
}

//Queen implementation ---------------------------------------------------------
// Reactive ant queen: generation info -----------------------------------------
struct AntQueenImpl<ReactiveAnt>::GenerationInfo {

  GenerationInfo(const AntHeader& header);

  std::shared_ptr<Ant> CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet);

  bool CanBeAdmitted(const AntHeader& header);
  void UpdateGenerationData(const AntHeader& header);
  bool IsBetterAnt(const AntHeader& header);
  bool HasRightAntType(const AntTypeHeader& typeHeader);


  uint32_t m_generation;
  uint8_t m_bestHopCount;
  Time m_bestTime;
};

AntQueenImpl<ReactiveAnt>::GenerationInfo::GenerationInfo(const AntHeader& header)
  : m_generation(header.GetGeneration()), m_bestHopCount(header.GetHopCount()), m_bestTime(header.GetTimeEstimate()) {
}

std::shared_ptr<Ant>
AntQueenImpl<ReactiveAnt>::GenerationInfo::CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) {
  AntHeader header;
  packet -> PeekHeader(header);

  if(!HasRightAntType(typeHeader) || !CanBeAdmitted(header)) {
    return nullptr;
  }

  UpdateGenerationData(header);
  return std::make_shared<ReactiveAnt>(packet);
}


AntType
AntQueenImpl<ReactiveAnt>::GetAntType() {
  return ReactiveAnt::species;
}


bool
AntQueenImpl<ReactiveAnt>::GenerationInfo::CanBeAdmitted(const AntHeader& header) {

  if(header.GetGeneration() < m_generation) {
    return false;
  }

  if(header.GetGeneration() > m_generation) {
    return true;
  }

  auto timeRatio = header.GetTimeEstimate() / m_bestTime;
  auto hopRatio  = header.GetHopCount() / m_bestHopCount;

  return timeRatio <= s_admissionRatio && hopRatio <= s_admissionRatio;
}

void
AntQueenImpl<ReactiveAnt>::GenerationInfo::UpdateGenerationData(const AntHeader& header) {

  if(header.GetGeneration() < m_generation) {
    return;
  }

  if(IsBetterAnt(header)) {
    m_generation = header.GetGeneration();
    m_bestHopCount = header.GetHopCount();
    m_bestTime = header.GetTimeEstimate();
  }

}


bool
AntQueenImpl<ReactiveAnt>::GenerationInfo::IsBetterAnt(const AntHeader& header) {
  if(header.GetGeneration() > m_generation) {
    return true;
  }

  if (header.GetGeneration() == m_generation) {
    return (header.GetHopCount() / m_bestHopCount + header.GetTimeEstimate() / m_bestTime);
  }

  return false;
}

bool
AntQueenImpl<ReactiveAnt>::GenerationInfo::HasRightAntType(const AntTypeHeader& typeHeader) {
  return AntQueenImpl<ReactiveAnt>::HasRightAntType(typeHeader);
}

// ReactiveQueen Pimpl ---------------------------------------------------------

struct AntQueenImpl<ReactiveAnt>::ReactiveQueenImpl {
  ReactiveQueenImpl();

  using GenInfoMapType = std::map<Ipv4Address, std::shared_ptr<GenerationInfo>>;
  // generation of the ants produced by the given queen
  uint32_t m_ownGeneration;
  // information about the generations of other recipients
  GenInfoMapType m_generationInfo;
};

AntQueenImpl<ReactiveAnt>::ReactiveQueenImpl::ReactiveQueenImpl()
  : m_ownGeneration(0), m_generationInfo(GenInfoMapType()) { }


// ReactiveQueen ---------------------------------------------------------------

// static variables and functions-----------------------------------------------
double AntQueenImpl<ReactiveAnt>::s_admissionRatio = 1.5;

bool
AntQueenImpl<ReactiveAnt>::HasRightAntType(const AntTypeHeader& typeHeader) {
  return ReactiveAnt::species == typeHeader.GetAntType();
}


// instance definitions --------------------------------------------------------
AntQueenImpl<ReactiveAnt>::AntQueenImpl() :
  m_impl(std::make_shared<ReactiveQueenImpl>()) {

}
AntQueenImpl<ReactiveAnt>::~AntQueenImpl() { }


std::shared_ptr<Ant> AntQueenImpl<ReactiveAnt>::CreateNew(Ipv4Address source, Ipv4Address dest) {
  auto generation  = m_impl -> m_ownGeneration ++;
  return std::make_shared<ReactiveAnt>(source, dest, generation);
}

std::shared_ptr<Ant> AntQueenImpl<ReactiveAnt>::CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) {

  if (!HasRightAntType(typeHeader)) {
    return nullptr;
  }

  AntHeader header;
  packet -> PeekHeader(header);

  auto optInfo = m_impl -> m_generationInfo.find(header.GetSource());
  // if there is an entry for the given source
  if(optInfo == m_impl -> m_generationInfo.end()) {
    auto info = optInfo -> second;
    return info->CreateFrom(typeHeader, packet);
  }else { // if no entry for a given sourceL create one
    auto info = std::make_shared<GenerationInfo>(header);
    m_impl -> m_generationInfo[header.GetSource()] = info;
    return std::make_shared<ReactiveAnt>(packet);
  }
}

} // namespace ant_routing
} // namespace ns3
