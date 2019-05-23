/**
 * Test suite for the ant-net devices.
 */
// header for the module under test
#include "ns3/ant-netdevice.h"
// An essential include is test.h
#include "ns3/test.h"
// other headers required for the test
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

// TODO also test for the case that the packages are dropped (see if the righ callbacks are made!)

namespace ns3 {
namespace ant_routing {

class AntNetDeviceTestCase1 : public TestCase {
public:
  AntNetDeviceTestCase1();
  virtual ~AntNetDeviceTestCase1() = default;
private:
  virtual void DoRun() override;
  void Call(Ipv4Address sourceAddr, Ipv4Address destAddr, AntNetDevice sourceAntDev, UnicastCallback ucb, Ptr<Ipv4> ipv4);
  void CounterCallback(Ptr<Ipv4Route> route, Ptr<const Packet> p, const Ipv4Header& header);
  uint32_t m_counter;
};

AntNetDeviceTestCase1::AntNetDeviceTestCase1()
  : TestCase("AntNetDevice test case, testing if the queueing mechanism works"), m_counter(0)
  {}

void AntNetDeviceTestCase1::DoRun() {
  std::size_t nodeCount = 2;
  double distance = 10;
  // first create the nodes
  NodeContainer nodes;
  nodes.Create(nodeCount);

  // then place the node in 2D space
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(0.0),
                                 "MinY", DoubleValue(0.0),
                                 "DeltaX", DoubleValue(distance),
                                 "DeltaY", DoubleValue(0),
                                 "GridWidth", UintegerValue(nodeCount),
                                 "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  NetDeviceContainer devices;
  devices = wifi.Install(wifiPhy, wifiMac, nodes);

  InternetStackHelper stack;
  stack.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (devices);

  // install the AntNetDevice on top of the devices, this is not their typical use
  // but is good enough to test the scheduling;
  auto SourcePair = interfaces.Get(0);
  auto DestPair = interfaces.Get(1);


  Ptr<NetDevice> sourceDev = SourcePair.first->GetNetDevice(SourcePair.second);
  Ptr<NetDevice> destDev = DestPair.first->GetNetDevice(DestPair.second);

  AntNetDevice sourceAntDev (sourceDev);
  AntNetDevice destAntDev (destDev);

  // // global routing is discouraged for wireless networks, but we placed the nodes
  // // close enough such that it will work :)
  // // + we do not get any noise on the tests from the routing protocol.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  UnicastCallback cb = MakeCallback(&AntNetDeviceTestCase1::CounterCallback, this);

  uint32_t packageCount(5); // do not go over 5 for testing purposes... packages will be dropped at IP layer!

  for(uint32_t i = 0; i < packageCount; i++) {
    Simulator::Schedule(Seconds(2), &AntNetDeviceTestCase1::Call, this, interfaces.GetAddress(0), interfaces.GetAddress(1), sourceAntDev, cb, SourcePair.first);
  }

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_TEST_ASSERT_MSG_EQ(m_counter, packageCount, "none of the packages should get lost");
  NS_TEST_ASSERT_MSG_NE(sourceAntDev.SendingTimeEst(), 0.0, "better estimate should be found");
}

void
AntNetDeviceTestCase1::Call(Ipv4Address sourceAddr,
                         Ipv4Address destAddr,
                         AntNetDevice sourceAntDev,
                         UnicastCallback ucb,
                         Ptr<Ipv4> ipv4) {
   Ptr<Packet> packet = Create<Packet>();
   Ipv4Header header;
   header.SetSource(sourceAddr);
   header.SetDestination(sourceAddr);
   Ipv4GlobalRoutingHelper helper;
   Socket::SocketErrno err;
   auto routeToDest = ipv4->GetRoutingProtocol() -> RouteOutput(packet, header, sourceAntDev.Device(), err);

   // place an entry in the queue of the ant device, will do a callback to the forwarder
   sourceAntDev.Submit(AntQueueEntry( Ptr<Ipv4Route>(), packet, header, ucb));
   // dest == next hop

   ipv4->Send(packet, sourceAddr, destAddr, 0, routeToDest);
}

void
AntNetDeviceTestCase1::CounterCallback(Ptr<Ipv4Route> route, Ptr<const Packet> p, const Ipv4Header& header) {
  m_counter++;
}



class AntNetDeviceTestSuite : public TestSuite {
public:
  AntNetDeviceTestSuite();
};

AntNetDeviceTestSuite::AntNetDeviceTestSuite ()
  : TestSuite("ant-netdevice", UNIT){

    AddTestCase(new AntNetDeviceTestCase1, TestCase::QUICK);

}

static AntNetDeviceTestSuite antNetDeviceTestSuite;

} // namespace ant_routing
} // namespace ns3
