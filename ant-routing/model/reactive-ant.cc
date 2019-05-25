#include "reactive-ant.h"
#include "ant-routing.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ReactiveAnt");

namespace ant_routing {

ReactiveAnt::ReactiveAnt(Ptr<Packet> packet) {
}

void ReactiveAnt::Visit(AnthocnetRouting router){
  //TODO implement
}

//Queen implementation ---------------------------------------------------------
double AntQueenImpl<ReactiveAnt>::s_admissionRatio = 1.5;

std::shared_ptr<Ant> AntQueenImpl<ReactiveAnt>::CreateFrom(const AntTypeHeader& typeHeader, Ptr<Packet> packet) {
  AntHeader header;
  packet -> PeekHeader(header);

  if(!HasRightAntType(typeHeader) || !CanBeAdmitted(header)) {
    return nullptr;
  }

  UpdateGenerationData(header);
  return std::make_shared<ReactiveAnt>(packet);
}

AntType AntQueenImpl<ReactiveAnt>::GetAntType() {
  return ReactiveAnt::species;
}


bool AntQueenImpl<ReactiveAnt>::CanBeAdmitted(const AntHeader& header) {

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

void AntQueenImpl<ReactiveAnt>::UpdateGenerationData(const AntHeader& header) {
  if(header.GetGeneration() < m_generation) {
    return;
  }

  if(IsBetterAnt(header)) {
    m_generation = header.GetGeneration();
    m_bestHopCount = header.GetHopCount();
    m_bestTime = header.GetTimeEstimate();
  }

}

bool AntQueenImpl<ReactiveAnt>::IsBetterAnt(const AntHeader& header) {
  if(header.GetGeneration() > m_generation) {
    return true;
  }

  if (header.GetGeneration() == m_generation) {
    return (header.GetHopCount() / m_bestHopCount + header.GetTimeEstimate() / m_bestTime);
  }

  return false;
}

bool AntQueenImpl<ReactiveAnt>::HasRightAntType(const AntTypeHeader& typeHeader) {
  return ReactiveAnt::species == typeHeader.GetAntType();
}



} // namespace ant_routing
} // namespace ns3
