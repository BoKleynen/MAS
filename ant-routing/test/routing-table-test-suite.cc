
// SUT header
#include "ns3/ant-routing-table.h"
// enable ns3 testing
#include "ns3/test.h"
#include "ns3/core-module.h"
#include <iostream>
namespace ns3 {
namespace ant_routing {

// Test case 1 -----------------------------------------------------------------
class AntRoutingTableTestCase1 : public TestCase {
public:
  AntRoutingTableTestCase1 ();
  virtual ~AntRoutingTableTestCase1() = default;
private:
  virtual void DoRun(void) override;
};

AntRoutingTableTestCase1::AntRoutingTableTestCase1()
  : TestCase("Routing table test case: creating and looking up routing table entries")
  {}

void AntRoutingTableTestCase1::DoRun(void) {
  // initialize variables needed to perform the test
  AntRoutingTable rt;

  // the table should be empty
  Ipv4Address neighbor("0.0.0.0");
  Ipv4Address destination("0.0.0.1");
  AntHeader ah;
  ah.SetOrigin(Ipv4Address("192.168.0.1"));
  ah.SetDestination(destination);
  // 0 == nullptr... wont compile otherwise :/
  NS_TEST_ASSERT_MSG_EQ(rt.GetPheromone(neighbor, destination), 0, "The table should be empty");

  // adding a new neighbor and retrieving values.
  rt.AddNeighbor(Neighbor(neighbor, Ptr<NetDevice>()));
  auto noPheromones = rt.NoPheromoneRoutes(ah);
  NS_TEST_ASSERT_MSG_EQ(noPheromones.size(), 1, "There should be no entries in the routing table");

  rt.SetPheromoneAt(neighbor, destination, PheromoneEntry(10, 0, Seconds(0)));
  auto pheromone = rt.GetPheromone(neighbor, destination);

  NS_TEST_ASSERT_MSG_EQ(pheromone->Value(), 10, "The table should contain an entry");

  noPheromones = rt.NoPheromoneRoutes(ah);
  NS_TEST_ASSERT_MSG_EQ(noPheromones.size(), 0, "The the selected destination should have a route");
}
// TestCase 2 ------------------------------------------------------------------
class AntRoutingTableTestCase2 : public TestCase {
public:
  AntRoutingTableTestCase2 ();
  virtual ~AntRoutingTableTestCase2() = default;
private:
  virtual void DoRun(void) override;
};


AntRoutingTableTestCase2::AntRoutingTableTestCase2()
  : TestCase("Routing table test case: creating and looking up routing table entries")
  {}

void AntRoutingTableTestCase2::DoRun() {
  Ipv4Address neighborAddr ("192.168.0.1");
  Ipv4Address destinationAddr ("192.168.0.2");
  Time estimate = Seconds(1);
  uint32_t hops = 10;
  AntRoutingTable rt;
  rt.AddNeighbor(Neighbor(neighborAddr, Ptr<NetDevice>()));

  NS_TEST_ASSERT_MSG_EQ(rt.HasPheromoneEntryFor(destinationAddr), false, "Should have no entry");
  NS_TEST_ASSERT_MSG_EQ(rt.IsNeighbor(neighborAddr), true, "The neighborAddress should be registered as a neighbor");
  NS_TEST_ASSERT_MSG_EQ(rt.IsNeighbor(destinationAddr), false, "The destination address is not a neighbor");

  rt.UpdatePheromoneEntry(neighborAddr, destinationAddr, estimate, hops);
  auto pheromonePtr = rt.GetPheromone(neighborAddr, destinationAddr);
  double expectedPheromone = (1- AntRoutingTable::Gamma())*2 / (estimate.GetSeconds() + hops*AntRoutingTable::HopTime().GetSeconds());
  NS_TEST_ASSERT_MSG_EQ_TOL(pheromonePtr->Value(), expectedPheromone, 10E-5 , "The pheromone entry should be updated");
  NS_TEST_ASSERT_MSG_EQ(pheromonePtr->HopCount(), hops, "The number of hops should be updated");
  NS_TEST_ASSERT_MSG_EQ(pheromonePtr->TimeEstimate(), estimate, "The time estimate should be updated");

  auto oldPheromoneValue = pheromonePtr -> Value();
  // test if the pheromone value is updated in place by checking against the pointer.
  rt.UpdatePheromoneEntry(neighborAddr, destinationAddr, estimate, hops);
  expectedPheromone = oldPheromoneValue * AntRoutingTable::Gamma() + (1 - AntRoutingTable::Gamma())*2/(estimate.GetSeconds() + hops*AntRoutingTable::HopTime().GetSeconds());
  NS_TEST_ASSERT_MSG_EQ_TOL(pheromonePtr -> Value(), expectedPheromone, 10E-5, "The pheromone value should be updated");
}

// Test suite setup ------------------------------------------------------------
class AntRoutingTableTestSuite : public TestSuite {
public:
  AntRoutingTableTestSuite();
};

AntRoutingTableTestSuite::AntRoutingTableTestSuite()
  : TestSuite ("ant-routing-table", UNIT) {
  //TestCases
  AddTestCase (new AntRoutingTableTestCase1, TestCase::QUICK);
  AddTestCase (new AntRoutingTableTestCase2, TestCase::QUICK)
;

}

static AntRoutingTableTestSuite antRoutingTableTestSuite;


}// namespace ant_routing
}// namespace ns3
