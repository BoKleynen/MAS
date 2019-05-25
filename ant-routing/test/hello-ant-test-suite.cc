#include "ns3/ant-routing.h"
#include "ns3/test.h"

// auxillary includes:
#include "ns3/core-module.h"
#include "ns3/ant-routing-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/v4ping-helper.h"

namespace ns3 {
namespace ant_routing {

class HelloRoutingTestCase1 : public TestCase {
public:
  HelloRoutingTestCase1();
  virtual ~HelloRoutingTestCase1() = default;
private:
  virtual void DoRun() override;
};

HelloRoutingTestCase1::HelloRoutingTestCase1()
  : TestCase("Hello routing test case where hello's are received regularily") { }

void HelloRoutingTestCase1::DoRun() {
  NS_LOG_UNCOND(AnthocnetRouting::TYPENAME);

  NS_LOG_UNCOND("TID: " << TypeId::LookupByName(AnthocnetRouting::TYPENAME));

  std::size_t nodeCount = 2;

  NodeContainer nodes;
  nodes.Create(nodeCount);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (10),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (nodeCount),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel (wifiChannel.Create());
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  NetDeviceContainer devices;
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  AnthocnetHelper anthocnet;
  InternetStackHelper stack;
  stack.SetRoutingHelper(anthocnet);
  stack.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (devices);

  AnthocnetRouting::SetHelloTimerInterval(MilliSeconds(3000));
  // SimpleFailureDetector::HelloInterval(MilliSeconds(3000));

  Simulator::Stop (Seconds(20));
  Simulator::Run();

  Ptr<AnthocnetRouting> routing = nodes.Get(0) -> GetObject<AnthocnetRouting>();
  auto nbCount = routing->GetRoutingTable().Neighbors().size();
  NS_TEST_ASSERT_MSG_EQ(nbCount, 1, "one entry should be added to the routing table");
  Simulator::Destroy();
}


class HelloMessageTestSuite : public TestSuite {
public:
  HelloMessageTestSuite();
};

HelloMessageTestSuite::HelloMessageTestSuite()
  : TestSuite("hello-ant", UNIT) {
  AddTestCase(new HelloRoutingTestCase1, TestCase::QUICK);
}

static HelloMessageTestSuite helloMessageTestSuite;


} // namespace ant_routing
} // namespace ns3
