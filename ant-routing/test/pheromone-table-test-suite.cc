/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/pheromone-table.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
namespace ns3 {
namespace ant_routing {

// This is an example TestCase.
class PheromoneTestCase1 : public TestCase
{
public:
  PheromoneTestCase1 ();
  virtual ~PheromoneTestCase1 ();

private:
  virtual void DoRun (void);
  PheromoneTable m_table;
};

// Add some help text to this case to describe what it is intended to test
PheromoneTestCase1::PheromoneTestCase1 ()
  : TestCase ("Pheromone test case"), m_table(PheromoneTable())
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
PheromoneTestCase1::~PheromoneTestCase1 ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
PheromoneTestCase1::DoRun (void)
{
  Ipv4Address addr1 ("0.0.0.1");
  Ipv4Address addr2 ("0.0.0.2");
  uint64_t hopTime = 1;
  uint32_t hops = 1;
  uint64_t timeEstimate = 1;
  double gamma = 0.7;

  PheromoneTable::SetHopTime(Seconds(hopTime));
  m_table.AddNeighbor(addr1);
  
  double pm = m_table.GetPheromoneFor(addr1, addr2);
  NS_TEST_ASSERT_MSG_EQ(pm, static_cast<double>(PheromoneTable::PheromoneInfo::defaultVal), "if there is no entry, the default value should be returned");
  auto ah = AntHeader();
  ah.SetOrigin(addr1);
  ah.SetDestination(addr2);
  auto noPheromone = m_table.GetNoPheromoneRoutes(ah);
  NS_TEST_ASSERT_MSG_EQ(noPheromone.size(), 1, "The neighbor should not have an entry for the destination");
  m_table.UpdateEntry(addr1, addr2, Seconds(timeEstimate), hops);
  noPheromone = m_table.GetNoPheromoneRoutes(ah);
  NS_TEST_ASSERT_MSG_EQ(noPheromone.size(), 0, "The neighbor should not have an empty entry for the destination");

  pm = m_table.GetPheromoneFor(addr1, addr2);
  NS_LOG_UNCOND("Expected: " << (1 - gamma) * 0.5*(timeEstimate + hopTime*hops));
  NS_LOG_UNCOND("Pheromone: " << pm);
  // NS_TEST_ASSERT_MSG_EQ_TOL(pm,
  //   (1 - gamma) * 0.5*(timeEstimate + hopTime*hops), 10,
  //    "It should be possible to update the pheromone value");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class PheromoneTestSuite : public TestSuite
{
public:
  PheromoneTestSuite ();
};

PheromoneTestSuite::PheromoneTestSuite ()
  : TestSuite ("pheromone-table", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new PheromoneTestCase1, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static PheromoneTestSuite pheromoneTestSuite;

}
}
