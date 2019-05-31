#include "link-failure-ant.h"
#include "ant-routing.h"
#include "ant-routing-table.h"


namespace ns3 {
namespace ant_routing {

bool LinkFailureAnt::s_pheromoneUpdatesOnFailure = true;

bool LinkFailureAnt::PheromoneUpdatesOnFailureEnabled() {
  return s_pheromoneUpdatesOnFailure;
}
void LinkFailureAnt::PheromoneUpdatesOnFailureEnabled(bool val) {
  s_pheromoneUpdatesOnFailure = val;
}



LinkFailureAnt::LinkFailureAnt(Ptr<Packet> packet)
  : m_header(LinkFailureNotification()){
  packet -> RemoveHeader(m_header);
}

void
LinkFailureAnt::Visit(AnthocnetRouting router) {

  NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now() << "-Received link failure notification" << m_header.m_messages.size());

  // do not forward if a loop is detected
  if (LoopDetection(router.GetAddress())) {
    NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now().GetSeconds() << "LOOP DETECTED!!!");
    return;
  }

  auto rTable = router.GetRoutingTable();

  std::vector<AlternativeRoute> altRoutes;
  for(auto messageIt = m_header.m_messages.begin(); messageIt != m_header.m_messages.end(); messageIt ++) {
    auto neighborAddr = m_header.m_source;
    auto dest = messageIt -> dest;

    // keep track of the fact that before the update, the neighbor was the best or not for a given destination
    auto wasBest = rTable.IsBestEntryFor(neighborAddr, dest);
    // std::cout<< "Was best?: " << wasBest << std::endl;

    //update if valid estimates, remove if not
    if(messageIt -> HasValidEstimates() && PheromoneUpdatesOnFailureEnabled()) {
      rTable.UpdatePheromoneEntry(neighborAddr, dest, messageIt -> bestTimeEstimate, messageIt -> bestHopEstimate);
    } else {
      rTable.DeletePheromoneEntryFor(neighborAddr, dest);
    }

    // rTable.DeletePheromoneEntryFor(neighborAddr, dest);

    // check if has lost the best entry for the destination
    if(wasBest && !rTable.IsBestEntryFor(neighborAddr, dest)) {
      NS_LOG_UNCOND("Best alternative: " << rTable.GetBestAlternativeFor(neighborAddr, messageIt -> dest).m_pheromone.HopCount());
      altRoutes.push_back(rTable.GetBestAlternativeFor(neighborAddr, messageIt -> dest));
    }
  }

  // do not forward if there are no best routes lost
  if (altRoutes.size() == 0){
    NS_LOG_UNCOND(router.GetAddress() << "@" << Simulator::Now().GetSeconds() << "Dropped Link failure notification, no lost best routes");
    return;
  }

  NextHop(router, altRoutes);

}

void
LinkFailureAnt::NextHop(AnthocnetRouting router, std::vector<AlternativeRoute> alternatives) {
  std::vector<LinkFailureNotification::Message> messages = ConvertAlternatives(alternatives);

  // for(auto altIt = alternatives.begin(); altIt != alternatives.end(); altIt++) {
  //   LinkFailureNotification::Message message;
  //   message.dest = altIt -> first.m_destination;
  //
  //   if(altIt -> second) {
  //     message.bestTimeEstimate = altIt -> first.m_pheromone.TimeEstimate();
  //     message.bestHopEstimate = altIt -> first.m_pheromone.HopCount();
  //   }
  //   message.SetValidEstimates(altIt -> second);
  //   messages.push_back(message);
  // }
  m_header.m_visitedNodes.push_back(router.GetAddress());
  LinkFailureNotification lfNotification(router.GetAddress(), m_header.m_visitedNodes, messages);
  auto packet = Create<Packet>();
  packet -> AddHeader(lfNotification);
  packet -> AddHeader(AntTypeHeader(species));

  BroadcastPacket(packet, router);
}

bool
LinkFailureAnt::LoopDetection(Ipv4Address addr) {

  for(auto iter = m_header.m_visitedNodes.begin(); iter != m_header.m_visitedNodes.end(); iter ++) {
    if(addr == *iter) {
      return true;
    }
  }

  return false;
}


// std::vector<std::pair<AlternativeRoute, bool>
// LinkFailureAnt::UpdateAndFindAlternatives(AnthocnetRouting router) {
//   std::vector<AlternativeRoute, bool> alt; // routes wherefore an alternative had to be chosen
//   auto rTable = router.GetRoutingTable();
//
//   for(auto messageIt = m_header.m_messages.begin(); messageIt = m_header.m_messages.end(); messageIt ++) {
//     auto neighborAddr = m_header.m_source;
//     auto dest = messgeIt -> dest;
//
//     if(rTable.IsBestEntryFor(neighborAddr, messageIt -> dest)){
//       alt.push_back(rTable.GetBestAlternativeFor(neighborAddr, messageIt -> dest));
//       rTable.UpdatePheromoneEntry(neighborAddr, dest, messageIt -> bestTimeEstimate, messageIt -> bestHopEstimate);
//     }
//   }
//
//   return alt;
// // }
//
//
// std::vector<std::pair<AlternativeRoute, bool>
// LinkFailureAnt::GetLostBests(AnthocnetRouting router) {
//   std::vector<AlternativeRoute, bool> altRoutes;
//   for(auto messageIt = m_header.m_messages.begin(); messageId = m_header.m_messages.end(); messageIt ++) {
//     auto neighborAddr = m_header.m_source;
//     auto dest = messageIt -> dest;
//
//     // keep track of the fact that before the update, the neighbor was the best or not for a given destination
//     auto wasBest = rTable.IsBestEntryFor(neighborAddr, dest));
//
//     // update if valid estimates, remove if not
//     if(messageIt -> HasValidEstimates()) {
//       rTable.UpdatePheromoneEntry(neighborAddr, dest, messageIt -> bestTimeEstimate, messageIt -> bestHopEstimate);
//     } else {
//       rTable.DeletePheromoneEntryFor(neighborAddr, dest);
//     }
//
//     // check if has lost the best entry for the destination
//     if(wasBest && !rTable.IsBestEntryFor(neighborAddr, dest)) {
//       altRoutes.push_back(rTable.GetBestAlternativeFor(neighborAddr, messageIt -> dest));
//     }
//   }
//   return altRoutes;
// }


Ptr<Packet>
LinkFailureAnt::ToPacket() {
  // TODO implement
  return Ptr<Packet>();
}

} // namespace ant_routing
} // namespace ns3
