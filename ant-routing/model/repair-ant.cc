#include "repair-ant.h"
#include "ant-routing.h"

namespace ns3 {
namespace ant_routing {

uint32_t RepairAnt::s_maxBroadcastCount = 2;

RepairAnt::RepairAnt(Ptr<Packet> packet)
  : ReactiveAnt(packet){ }


RepairAnt::RepairAnt(Ipv4Address source, Ipv4Address dest, uint32_t generation)
  : ReactiveAnt(source, dest, generation) {
  m_header.AddVisitedNode(source);
  m_header.m_hopCount++;

  NS_LOG_UNCOND("Created repair ant source: " << source << " dest: " << dest);
}

bool
RepairAnt::HandleBroadcast(AnthocnetRouting router) {
  if(m_header.m_broadcastCount > GetMaxBroadcastCount()) {
    return false;
  }

  m_header.m_broadcastCount ++;
  return this -> ReactiveAnt::HandleBroadcast(router);
}

uint32_t
RepairAnt::GetMaxBroadcastCount() {
  return s_maxBroadcastCount;
}

void
RepairAnt::SetMaxBroadcastCount(uint32_t count) {
  s_maxBroadcastCount = count;
}

Ptr<Packet>
RepairAnt::ToPacket() {
  return Ptr<Packet>();
}

struct AntQueenImpl<RepairAnt>::GenerationInfo {

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

AntQueenImpl<RepairAnt>::GenerationInfo::GenerationInfo(const AntHeader& header)
  : m_generation(header.GetGeneration()), m_bestHopCount(header.GetHopCount()), m_bestTime(header.GetTimeEstimate()) {
}

std::shared_ptr<Ant>
AntQueenImpl<RepairAnt>::GenerationInfo::CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) {
  AntHeader header;
  packet -> PeekHeader(header);

  if(!HasRightAntType(typeHeader) || !CanBeAdmitted(header)) {
    return nullptr;
  }

  UpdateGenerationData(header);
  return std::make_shared<RepairAnt>(packet);
}


AntType
AntQueenImpl<RepairAnt>::GetAntType() {
  return RepairAnt::species;
}


bool
AntQueenImpl<RepairAnt>::GenerationInfo::CanBeAdmitted(const AntHeader& header) {
  NS_LOG_UNCOND("Origin of the ant: " << header.GetSource());
  NS_LOG_UNCOND("Generation of ant: " << header.GetGeneration() << " Current highest: " << m_generation);
  NS_LOG_UNCOND("Time of ant: " << header.GetTimeEstimate() <<" best time: " << m_bestTime);
  NS_LOG_UNCOND("Hop count ant: "<< (uint32_t)header.GetHopCount()<< "best count: " << static_cast<uint32_t>(m_bestHopCount));

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
AntQueenImpl<RepairAnt>::GenerationInfo::UpdateGenerationData(const AntHeader& header) {

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
AntQueenImpl<RepairAnt>::GenerationInfo::IsBetterAnt(const AntHeader& header) {
  if(header.GetGeneration() > m_generation) {
    NS_LOG_UNCOND("Better ant found by generation for" << header.GetSource() <<" - old gen: " << m_generation << "new gen: " << header.GetGeneration());
    return true;
  }

  if (header.GetGeneration() == m_generation) {
    return (header.GetHopCount() / m_bestHopCount + header.GetTimeEstimate() / m_bestTime);
  }

  return false;
}

bool
AntQueenImpl<RepairAnt>::GenerationInfo::HasRightAntType(const AntTypeHeader& typeHeader) {
  return AntQueenImpl<RepairAnt>::HasRightAntType(typeHeader);
}

// ReactiveQueen Pimpl ---------------------------------------------------------

struct AntQueenImpl<RepairAnt>::ReactiveQueenImpl {
  ReactiveQueenImpl();

  using GenInfoMapType = std::map<Ipv4Address, std::shared_ptr<GenerationInfo>>;
  // generation of the ants produced by the given queen
  uint32_t m_ownGeneration;
  // information about the generations of other recipients
  GenInfoMapType m_generationInfo;
};

AntQueenImpl<RepairAnt>::ReactiveQueenImpl::ReactiveQueenImpl()
  : m_ownGeneration(0), m_generationInfo(GenInfoMapType()) { }


// ReactiveQueen ---------------------------------------------------------------

// static variables and functions-----------------------------------------------
double AntQueenImpl<RepairAnt>::s_admissionRatio = 1.5;

bool
AntQueenImpl<RepairAnt>::HasRightAntType(const AntTypeHeader& typeHeader) {
  return RepairAnt::species == typeHeader.GetAntType();
}


// instance definitions --------------------------------------------------------
AntQueenImpl<RepairAnt>::AntQueenImpl() :
  m_impl(std::make_shared<ReactiveQueenImpl>()) {

}
AntQueenImpl<RepairAnt>::~AntQueenImpl() { }


std::shared_ptr<Ant> AntQueenImpl<RepairAnt>::CreateNew(Ipv4Address source, Ipv4Address dest) {
  auto generation  = ++(m_impl -> m_ownGeneration); // start out with the next generation (counters start at zero)
  return std::make_shared<RepairAnt>(source, dest, generation);
}

std::shared_ptr<Ant> AntQueenImpl<RepairAnt>::CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) {

  if (!HasRightAntType(typeHeader)) {
    return nullptr;
  }

  AntHeader header;
  packet -> PeekHeader(header);

  auto optInfo = m_impl -> m_generationInfo.find(header.GetSource());
  // if there is an entry for the given source
  if(optInfo != m_impl -> m_generationInfo.end()) {
    auto info = optInfo -> second;
    return info->CreateFrom(typeHeader, packet);
  }else { // if no entry for a given sourceL create one
    auto info = std::make_shared<GenerationInfo>(header);
    NS_LOG_UNCOND("Created new geninfo with: best time" << info -> m_bestTime << " and generation: " << info -> m_generation);
    m_impl -> m_generationInfo[header.GetSource()] = info;
    return std::make_shared<RepairAnt>(packet);
  }
}

double
AntQueenImpl<RepairAnt>::AdmissionRatio() {
  return s_admissionRatio;
}
void
AntQueenImpl<RepairAnt>::AdmissionRatio(double ratio) {
  if(ratio < 0) {
    return;
  }

  s_admissionRatio = ratio;
}



} // namespace ns3
} // namespace ant_routing
