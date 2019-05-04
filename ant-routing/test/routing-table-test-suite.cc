
// SUT header
#include "ns3/ant-routing-table.h"
// enable ns3 testing
#include "ns3/test.h"
#include "ns3/core-module.h"
#include <iostream>
namespace ns3 {
namespace ant_routing {

// Define a dummy entry --------------------------------------------------------


struct RtTestEntry {
  RtTestEntry();
  void Update(int i);
  int m_int;
};

bool operator==(const RtTestEntry& lhs, const RtTestEntry& rhs);


void RtTestEntry::Update(int i) {
  m_int += i;
}

RtTestEntry::RtTestEntry() : m_int(0) {

}

bool operator==(const RtTestEntry& lhs, const RtTestEntry& rhs){
  return lhs.m_int == rhs.m_int;
}

bool operator!=(const RtTestEntry& lhs, const RtTestEntry& rhs){
  return !(lhs == rhs);
}

std::ostream&  operator <<(std::ostream& os, const RtTestEntry& entry) {
  return os << "RtTestEntry { m_int: " << entry.m_int << "}";
}

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
  RoutingTable<RtTestEntry> rt;
  Ipv4Address addr ("0.0.0.0");
  RtTestEntry entry;
  int myInt = 7;

  // do the tests
  NS_TEST_ASSERT_MSG_EQ(rt.HasEntry(addr), false, "The routing table should not yet contain the address");

  rt.Insert(addr, entry);
  NS_TEST_ASSERT_MSG_EQ(rt.HasEntry(addr), true, "The routing table should contain the addrress");
  Optional<RtTestEntry> lookupEntry = rt.Lookup(addr);
  NS_TEST_ASSERT_MSG_EQ(entry, lookupEntry.ref(), "The routing table should contain the same entry as was previously added");
  rt.Update(addr, myInt);
  NS_TEST_ASSERT_MSG_NE(entry, lookupEntry.ref(), "Adding an entry to the routing table has copy semantics");
  lookupEntry = rt.Lookup(addr);
  NS_TEST_ASSERT_MSG_EQ(lookupEntry.ref().m_int, myInt, "References to the entry should be updated");
  rt.Delete(addr);
  NS_TEST_ASSERT_MSG_EQ(rt.HasEntry(addr), false, "The entry should be removed");
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
;

}

static AntRoutingTableTestSuite antRoutingTableTestSuite;


}// namespace ant_routing
}// namespace ns3
