#include "reactive-ant.h"
#include "ant-routing.h"
#include "ant-routing-table.h"
#include "ant-netdevice.h"
#include "ant-hill.h"
#include "backward-ant.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ReactiveAnt");

namespace ant_routing {

ReactiveAnt::ReactiveAnt(Ipv4Address source, Ipv4Address destination, uint32_t generation)
  : m_header(AntHeader()){
  m_header.SetSource(source);
  m_header.SetDestination(destination);
  m_header.SetGeneration(generation);

  NS_LOG_UNCOND("Created reactive ant");
}

ReactiveAnt::ReactiveAnt(Ptr<Packet> packet)
  : m_header(AntHeader()){

  packet -> RemoveHeader(m_header);
  NS_LOG_UNCOND("received reactive ant");
}

void ReactiveAnt::Visit(AnthocnetRouting router){
  NS_LOG_UNCOND("visiting node with forward ant");

  if(HandleAtDestination(router)){
    return;
  }

  // geather data on the router
  if(HandleBroadcast(router)){
      return;
  }

  if(HandleUnicast(router)) {
    return;
  }
  // error
}

bool
ReactiveAnt::HandleAtDestination(AnthocnetRouting router) {
  // only continue if we're at the destination
  if(GetHeader().GetDestination() != router.GetAddress()) {
    return false;
  }

  auto bwAnt = router.GetAntHill().Get<BackwardQueen>()->CreateFromForwardAnt(GetHeader());
  bwAnt -> Visit(router);

  return true;
}

bool
ReactiveAnt::HandleBroadcast(AnthocnetRouting router){
  auto routingTable = router.GetRoutingTable();
  if(routingTable.HasPheromoneEntryFor(GetHeader().GetDestination())) {
    return false;
  }

  auto packet = NextHopPacket(router);

  BroadcastPacket(packet, router);

  return true;
}

bool
ReactiveAnt::HandleUnicast(AnthocnetRouting router) {
  auto routingTable = router.GetRoutingTable();
  if(!routingTable.HasPheromoneEntryFor(GetHeader().GetDestination())) {
    return false;
  }

  auto optNeighbor = routingTable.RouteAnt(GetHeader());
  if(!optNeighbor.IsValid()) {
    return false;
  }
  auto neighbor = optNeighbor.Get();

  auto packet = NextHopPacket(router);

  UnicastPacket(packet, router, neighbor);
  return true;
}


Ptr<Packet>
ReactiveAnt::NextHopPacket(AnthocnetRouting router) {
  AntHeader antHeader = GetHeader();
  antHeader.AddVisitedNode(router.GetAddress());
  antHeader.m_hopCount++;
  auto device = router.GetDevice();
  antHeader.m_timeEstimate = (device.QueueSize() + 1)*device.SendingTimeEst();
  auto packet = Create<Packet>();
  packet -> AddHeader(antHeader);
  packet -> AddHeader(AntTypeHeader(species));

  return packet;
}

Ptr<Packet>
ReactiveAnt::ToPacket() {
  return Ptr<Packet>();
}


const AntHeader&
ReactiveAnt::GetHeader() {
  return m_header;
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
    NS_LOG_UNCOND("Rejected!");
    return nullptr;
  }
  NS_LOG_UNCOND("ACCEPTED!!!!!!!!!!");

  UpdateGenerationData(header);
  return std::make_shared<ReactiveAnt>(packet);
}


AntType
AntQueenImpl<ReactiveAnt>::GetAntType() {
  return ReactiveAnt::species;
}


bool
AntQueenImpl<ReactiveAnt>::GenerationInfo::CanBeAdmitted(const AntHeader& header) {
  NS_LOG_UNCOND("Origin of the ant: " << header.GetSource());
  NS_LOG_UNCOND("Generation of ant: " << header.GetGeneration() << "Current highest: " << m_generation);
  NS_LOG_UNCOND("Time of ant: " << header.GetTimeEstimate() <<"best time: " << m_bestTime);
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
  auto generation  = ++(m_impl -> m_ownGeneration); // start out with the next generation (counters start at zero)
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
  if(optInfo != m_impl -> m_generationInfo.end()) {
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
