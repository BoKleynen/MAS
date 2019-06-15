#include "ns3/ant-routing-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/aodv-module.h"
using namespace ns3;
using namespace ant_routing;
using namespace aodv;
int main (int argc, char *argv[]) {
  NS_LOG_UNCOND(AnthocnetRouting::TYPENAME);
  NS_LOG_UNCOND("TID: " << TypeId::LookupByName(AnthocnetRouting::TYPENAME));
  Time simulationTime = Seconds(50);
  std::size_t nodeCount = 4;
  NodeContainer nodes;
  nodes.Create(nodeCount);
  MobilityHelper mobility;
  auto alloc = Create<ListPositionAllocator>();
  alloc -> Add(Vector(0  , 100, 0));
  alloc -> Add(Vector(50 , 150, 0));
  alloc -> Add(Vector(50 , 50, 0));
  alloc -> Add(Vector(125, 100, 0));
  mobility.SetPositionAllocator(alloc);
  //
  // mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                                "MinX", DoubleValue (0.0),
  //                                "MinY", DoubleValue (0.0),
  //                                "DeltaX", DoubleValue (100),
  //                                "DeltaY", DoubleValue (0),
  //                                "GridWidth", UintegerValue (nodeCount),
  //                                "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  // NodeContainer mobileNodes;
  // mobileNodes.Create(1);
  // MobilityHelper movingMobility;
  // auto mobileAlloc = Create<ListPositionAllocator>();
  // mobileAlloc -> Add(Vector(130, 0, 0));
  // movingMobility.SetPositionAllocator(mobileAlloc);
  // // waypoint model set
  // movingMobility.SetMobilityModel("ns3::WaypointMobilityModel");
  // movingMobility.Install(mobileNodes);
  // // add waypoints to the model
  // auto waypointMobility = DynamicCast<WaypointMobilityModel>(mobileNodes.Get(0) -> GetObject<MobilityModel>());
  // waypointMobility -> AddWaypoint(Waypoint(Seconds(2), Vector(140, 0, 0)));
  // waypointMobility -> AddWaypoint(Waypoint(simulationTime, Vector(250, 0, 0)));
  //
  // auto nodes = NodeContainer(staNodes, mobileNodes);
  //nodes.Add(mobileNodes.Get(0)); // aggregate the nodes for further use;
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
                                  "MaxRange", DoubleValue (100.0));
  wifiPhy.SetChannel (wifiChannel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  NetDeviceContainer devices;
  devices = wifi.Install (wifiPhy, wifiMac, nodes);
  AnthocnetHelper anthocnet;
  // AodvHelper anthocnet;
  InternetStackHelper stack;
  stack.SetRoutingHelper(anthocnet);
  stack.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (devices);
  V4PingHelper ping(interfaces.GetAddress(nodes.GetN() - 1));
  ping.SetAttribute("Verbose", BooleanValue(true));
  ping.SetAttribute("Interval", TimeValue(MilliSeconds(200)));
  //
  ApplicationContainer p = ping.Install(nodes.Get(0));
  p.Start(Seconds(0));
  p.Stop(simulationTime - Seconds(0.001));
  FlowMonitorHelper monitorHelper;
  auto monitor = monitorHelper.Install(nodes);
  Simulator::Stop (simulationTime);
  AnimationInterface anim ("bridge.netanim");
  // for (uint32_t i = 0; i < nodes.GetN (); i++)
  // {
  //   anim.UpdateNodeDescription (nodes.Get (i), "SEND"); // Optional
  //   anim.UpdateNodeColor (nodes.Get (i), 255, 0, 0); // Optional
  // }
  //
  // for (uint32_t i = 0; i < nodes.GetN (); i++)
  // {
  //   anim.UpdateNodeDescription (nodes.Get (i), "RECV"); // Optional
  //   anim.UpdateNodeColor (nodes.Get (i), 0, 255, 0); // Optional
  // }
  anim.EnablePacketMetadata (); // Optional
  anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
  // anim.EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
  // anim.EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional
  Simulator::Run();
  auto router = nodes.Get(0) -> GetObject<AnthocnetRouting>();
  // auto pheromone1 = router->GetRoutingTable().GetPheromone(Ipv4Address("10.0.0.2"), Ipv4Address("10.0.0.4"));
  // auto pheromone2 = router->GetRoutingTable().GetPheromone(Ipv4Address("10.0.0.3"), Ipv4Address("10.0.0.4"));
  // std::cout << "Pheromone at end of simulation for 10.0.0.4 from 10.0.0.1 via 10.0.0.2: " << pheromone1 -> Value() << std::endl;
  // std::cout << "Pheromone at end of simulation for 10.0.0.4 from 10.0.0.1 via 10.0.0.3: " << pheromone2 -> Value() << std::endl;
  monitor->CheckForLostPackets ();
  monitor->SerializeToXmlFile ("bridge.flowmon", true, true);
  Simulator::Destroy();
}