/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "ns3/ant-packet.h"

// enable ns3 testing
#include "ns3/test.h"

using namespace ns3;
using namespace ant_routing;

class MessageDeserializeTestCase1 : public TestCase
{
public:
   MessageDeserializeTestCase1 ();
   virtual ~MessageDeserializeTestCase1() = default;
private:
  virtual void DoRun(void) override;
};

MessageDeserializeTestCase1::MessageDeserializeTestCase1 ()
  : TestCase("Routing table test case: creating and looking up routing table entries")
{
}

void
MessageDeserializeTestCase1::DoRun(void)
{
  LinkFailureNotification::Message message;
  message.bestHopEstimate = 123;
  message.dest = Ipv4Address ("127.0.0.1");
  message.bestTimeEstimate = Time ();


  auto buf = Buffer ();
  auto size = LinkFailureNotification::Message::GetSerializedSize ();
  buf.AddAtStart (size);
  message.Serialize (buf.Begin ());
  LinkFailureNotification::Message new_message;
  new_message.Deserialize (buf.Begin ());

  NS_TEST_ASSERT_MSG_EQ(message, new_message, "The deserialized message should be equal to the message that was serialized.");
}

class MessageDeserializeTestSuite : public TestSuite
{
public:
  MessageDeserializeTestSuite ();
};

MessageDeserializeTestSuite::MessageDeserializeTestSuite ()
  : TestSuite("ant-packet", UNIT)
{
  AddTestCase (new MessageDeserializeTestCase1, TestCase::QUICK);
}

static MessageDeserializeTestSuite testSuite;
