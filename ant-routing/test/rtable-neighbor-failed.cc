#include "ns3/ant-routing-table.h"
// enable ns3 testing
#include "ns3/test.h"
#include "ns3/core-module.h"
#include <iostream>

namespace ns3 {
namespace ant_routing {

class RTableFailureTestCase1 : public TestCase {
public:
  RTableFailureTestCase1();
  virtual ~RTableFailureTestCase1() = default;
private:
  virtual void DoRun();
};

RTableFailureTestCase1::RTableFailureTestCase1 ()
  : TestCase("routing table test case for updating in case of failure"){
}

void RTableFailureTestCase1::DoRun() {
  Ipv4Address nb1("0.0.0.1");
  Ipv4Address nb2("0.0.0.2");
  Ipv4Address nb3("0.0.0.3");
  Ipv4Address nb4("0.0.0.4");
  Ipv4Address dest("0.0.0.5");

  AntRoutingTable rt;
  rt.AddNeighbor(Neighbor(nb1, Ptr<NetDevice>()));
  rt.AddNeighbor(Neighbor(nb2, Ptr<NetDevice>()));
  rt.AddNeighbor(Neighbor(nb3, Ptr<NetDevice>()));
  rt.AddNeighbor(Neighbor(nb4, Ptr<NetDevice>()));

  rt.SetPheromoneAt(nb1, dest, PheromoneEntry(1, 5, MilliSeconds(100)));
  rt.SetPheromoneAt(nb2, dest, PheromoneEntry(2, 4, MilliSeconds(90)));
  rt.SetPheromoneAt(nb3, dest, PheromoneEntry(3, 3, MilliSeconds(80)));
  rt.SetPheromoneAt(nb4, dest, PheromoneEntry(4, 2, MilliSeconds(70)));

  NS_TEST_ASSERT_MSG_EQ(rt.IsBestEntryFor(nb4, dest), true, "fourth should be best");
  NS_TEST_ASSERT_MSG_EQ(rt.IsBestEntryFor(nb1, dest), false, "first should not");

  auto alt = rt.GetBestAlternativeFor(nb4, dest);
  NS_TEST_ASSERT_MSG_EQ(alt.m_neighbor.Address(), nb3, "Three is the best alternative for four");

  auto alts = rt.BestAlternativesFor(Neighbor(nb4, Ptr<NetDevice>()));
  NS_TEST_ASSERT_MSG_EQ(alts.size(), 1, "There should only be one alternative");
  NS_TEST_ASSERT_MSG_EQ(alts[0].m_neighbor.Address(), nb3, "Three should still be the best alternative for four");
}

class RTableFailureTestSuite : public TestSuite {
public:
  RTableFailureTestSuite();
};

RTableFailureTestSuite::RTableFailureTestSuite()
  : TestSuite("rtable-failure", UNIT) {
    AddTestCase(new RTableFailureTestCase1, TestCase::QUICK);
  }

static RTableFailureTestSuite rTableFalureTestSuite;

} // namespace ant_routing
} // namespace ns3
