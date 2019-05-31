#include <numeric>
#include "manet-routing-compare.h"
#include "ns3/ant-routing-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

int
main (int argc, char *argv[])
{
  std::vector<Scenario> scenarios;
  scenarios.push_back (Scenario (20, 450));
  scenarios.push_back (Scenario (25, 500));
  scenarios.push_back (Scenario (30, 550));
  scenarios.push_back (Scenario (35, 590));
  scenarios.push_back (Scenario (40, 630));
  scenarios.push_back (Scenario (45, 670));
  // m_scenarios.push_back (Scenario (50, 750));
  // m_scenarios.push_back (Scenario (75, 875));
  // m_scenarios.push_back (Scenario (100, 1000));
  // m_scenarios.push_back (Scenario (125, 1125));
  // m_scenarios.push_back (Scenario (150, 1250));

  std::vector<int> protocols;
  // protocols.push_back(2);
  protocols.push_back(4);

  for (auto protocol : protocols)
  {
    for (auto scenario : scenarios)
    {
      RoutingExperimentSuite experimentSuite (10, scenario, protocol);
      experimentSuite.RunSuite ();
      std::cout << experimentSuite.GetResult () << std::endl;
    }
  }
}

// RoutingExperimentSuite -------------------------

RoutingExperimentSuite::RoutingExperimentSuite (uint8_t nSimulations, Scenario scenario, int protocol)
  : m_nSimulations (nSimulations),
    m_scenario (scenario),
    m_protocol (protocol)
{
  switch (protocol)
  {
  case 2:
    m_protocolName = "AODV";
    break;
  case 4:
    m_protocolName = "Anthocnet";
    break;
  default:
    m_protocolName = "invalid";
    break;
  }
}

void
RoutingExperimentSuite::RunSuite ()
{
  int nSinks = 5;

  for (int i = 0; i < m_nSimulations; i++)
  {
    ns3::RngSeedManager::SetRun(i);        
    RoutingExperiment experiment (m_protocol, nSinks, m_scenario);
    experiment.Run ();
    m_results.push_back (experiment.GetResult ());
  }
}

Result
RoutingExperimentSuite::GetResult () const
{
  Result result;
  result.nNodes = m_scenario.nNodes;
  result.protocolName = m_protocolName;
  result.averageDelay = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.averageDelay;
  }) / m_results.size ();
  result.averageJitter = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.averageJitter;
  }) / m_results.size ();
  result.packetDeliveryRatio = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.packetDeliveryRatio;
  }) / m_results.size ();
  result.throughput = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.throughput;
  }) / m_results.size ();
  result.packetOverhead = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.packetOverhead;
  }) / m_results.size ();
  result.byteOverhead = std::accumulate(m_results.begin(), m_results.end(), 0.0, [] (double a, const Result& b) {
    return a + b.byteOverhead;
  }) / m_results.size ();

  return result;
}

// RoutingExperiment ------------------------------

// RoutingExperiment::RoutingExperiment ()
//   : port (3001),
//     bytesTotal (0),
//     packetsReceived (0),
//     m_traceMobility (false),
//     m_protocol (2), // AODV
//     m_txp(7.5)
// {
// }

RoutingExperiment::RoutingExperiment (uint32_t protocol, int nSinks, Scenario scenario)
: port (3001),
  bytesTotal (0),
  packetsReceived (0),
  m_traceMobility (false),
  m_protocol (protocol),
  m_txp (2),
  m_nSinks (nSinks),
  m_scenario (scenario)
{
}

void
RoutingExperiment::Run ()
{
  Packet::EnablePrinting ();
  Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));
  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (200));

  SetupNodes ();
  SetupWifi ();
  SetupMobility ();
  SetupProtocol ();
  SetupAddresses ();
  SetupApplications ();
  SetupFlowmon ();

  NS_LOG_INFO ("Configure Tracing.");
  auto tr_name = TraceName ();

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
  wifiPhy.EnableAsciiAll (osw);
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));

  NS_LOG_INFO ("Run Simulation.");

  // CheckThroughput ();
  AnimationInterface anim (tr_name + ".netanim");

  for (uint32_t i = 0; i < m_senderNodes.GetN (); i++)
  {
    anim.UpdateNodeDescription (m_senderNodes.Get (i), "SEND"); // Optional
    anim.UpdateNodeColor (m_senderNodes.Get (i), 255, 0, 0); // Optional
  }

  for (uint32_t i = 0; i < m_receiverNodes.GetN (); i++)
  {
    anim.UpdateNodeDescription (m_receiverNodes.Get (i), "RECV"); // Optional
    anim.UpdateNodeColor (m_receiverNodes.Get (i), 0, 255, 0); // Optional
  }

  anim.EnablePacketMetadata (); // Optional
  anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
  anim.EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
  anim.EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional

  Simulator::Stop (Seconds (TotalTime));
  Simulator::Run ();
  m_flowmon->CheckForLostPackets ();
  m_flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), true, true);

  Simulator::Destroy ();
}

Result
RoutingExperiment::GetResult ()
{
  Result result;
  result.protocolName = m_protocolName;
  result.nNodes = m_allNodes.GetN ();
  result.throughput = 0;

  double totalAverageDelay = 0;
  double totalAverageJitter = 0;
  int nDataFlows = 0;
  double rxDataPackets = 0;
  double txDataPackets = 0;
  double controlPackets = 0;
  double controlBytes = 0;
  double dataBytes = 0;

  FlowMonitor::FlowStatsContainer stats = m_flowmon->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (m_flowmonHelper.GetClassifier ());
  for (auto iter = stats.begin (); iter != stats.end (); iter++)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
    auto flowStats = iter->second;

    if (t.destinationPort == port) {
      nDataFlows++;
      totalAverageDelay += HistHelper (flowStats.delayHistogram).Average ();
      totalAverageJitter += HistHelper (flowStats.jitterHistogram).Average ();
      result.throughput += static_cast<double> (flowStats.rxBytes) / static_cast<double> (flowStats.timeLastRxPacket.GetSeconds () - flowStats.timeFirstTxPacket.GetSeconds ()) / 1024.0 * 8.0;
      txDataPackets += static_cast<double> (flowStats.txPackets);
      rxDataPackets += static_cast<double> (flowStats.rxPackets);
      dataBytes += static_cast<double> (flowStats.rxBytes);
    } else {
      controlPackets += flowStats.txPackets;
      controlBytes += flowStats.txBytes;
    }
  }

  result.averageDelay = totalAverageDelay / nDataFlows;
  result.averageJitter = totalAverageJitter / nDataFlows;
  result.packetDeliveryRatio = rxDataPackets / txDataPackets * 100;
  result.packetOverhead = controlPackets / rxDataPackets;
  result.byteOverhead = controlBytes / dataBytes;

  NS_ASSERT (nDataFlows == m_nSinks);

  return result;
}

static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}

void
RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}

// void
// RoutingExperiment::CheckThroughput ()
// {
//   double kbs = (bytesTotal * 8.0) / 1000;
//   bytesTotal = 0;

//   std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

//   out << (Simulator::Now ()).GetSeconds () << ","
//       << kbs << ","
//       << packetsReceived << ","
//       << m_nSinks << ","
//       << m_protocolName << ","
//       << m_txp << ""
//       << std::endl;

//   out.close ();
//   packetsReceived = 0;
//   Simulator::Schedule (Seconds (1.0), &RoutingExperiment::CheckThroughput, this);
// }

Ptr<Socket>
RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

  return sink;
}

// std::string
// RoutingExperiment::CommandSetup (int argc, char **argv)
// {
//   CommandLine cmd;
//   cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
//   cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
//   cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
//   cmd.Parse (argc, argv);
//   return m_CSVfileName;
// }

void
RoutingExperiment::SetupNodes ()
{
  NS_LOG_INFO ("Setting up nodes.");
  m_adhocNodes.Create (m_scenario.nNodes - 2*m_nSinks);
  m_senderNodes.Create (m_nSinks);
  m_receiverNodes.Create (m_nSinks);
  m_allNodes.Add (m_adhocNodes);
  m_allNodes.Add (m_senderNodes);
  m_allNodes.Add (m_receiverNodes);
}

void
RoutingExperiment::SetupWifi ()
{
  NS_LOG_INFO ("Setting up WiFi.");
  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  m_adhocDevices = wifi.Install (wifiPhy, wifiMac, m_adhocNodes);
  m_senderDevices = wifi.Install (wifiPhy, wifiMac, m_senderNodes);
  m_receiverDevices = wifi.Install (wifiPhy, wifiMac, m_receiverNodes);
  m_allDevices.Add (m_adhocDevices);
  m_allDevices.Add (m_senderDevices);
  m_allDevices.Add (m_receiverDevices);
}

void
RoutingExperiment::SetupMobility ()
{
  MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  std::stringstream ss;
  ss << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_scenario.size << "]";
  std::string s = ss.str ();
  pos.Set ("X", StringValue (s));
  pos.Set ("Y", StringValue (s));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=10.0]"),
                                  "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=10.0]"),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (m_allNodes);
  streamIndex += mobilityAdhoc.AssignStreams (m_allNodes, streamIndex);
  NS_UNUSED (streamIndex); // From this point, streamIndex is unused
}

void
RoutingExperiment::SetupProtocol ()
{
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  AnthocnetHelper anthocnet;

  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

  switch (m_protocol)
    {
    // case 1:
    //   list.Add (olsr, 100);
    //   m_protocolName = "OLSR";
    //   break;
    case 2:
      list.Add (aodv, 100);
      m_protocolName = "AODV";
      break;
    // case 3:
    //   list.Add (dsdv, 100);
    //   m_protocolName = "DSDV";
    //   break;
    case 4:
      // anthocnet
      list.Add (anthocnet, 100);
      m_protocolName = "AntHocNet";
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
    }

  internet.SetRoutingHelper (list);
  internet.Install (m_allNodes);
}

void
RoutingExperiment::SetupAddresses ()
{
  NS_LOG_INFO ("assigning ip address");

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  m_adhocInterfaces = address.Assign (m_adhocDevices);
  m_senderInterfaces = address.Assign (m_senderDevices);
  m_receiverInterfaces = address.Assign (m_receiverDevices);
}

void
RoutingExperiment::SetupApplications ()
{
  NS_LOG_INFO ("Setting applications.");

  OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address ());
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

  for (uint32_t i = 0; i < m_receiverDevices.GetN (); i++)
  {
    Ptr<Socket> sink = SetupPacketReceive (m_receiverInterfaces.GetAddress (i), m_receiverNodes.Get (i));

    AddressValue remoteAddress (InetSocketAddress (m_receiverInterfaces.GetAddress (i), port));
    onoff1.SetAttribute ("Remote", remoteAddress);

    Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
    ApplicationContainer temp = onoff1.Install (m_senderNodes.Get (i));
    temp.Start (Seconds (var->GetValue (100.0,101.0)));
    temp.Stop (Seconds (TotalTime));
  }
}

std::string
RoutingExperiment::TraceName ()
{
  std::string tr_name ("manet-routing-compare");
  std::stringstream ss;
  ss << m_allNodes.GetN ();
  std::string nodes = ss.str ();

  std::stringstream ss2;
  ss2 << nodeSpeed;
  std::string sNodeSpeed = ss2.str ();

  std::stringstream ss3;
  ss3 << nodePause;
  std::string sNodePause = ss3.str ();

  std::stringstream ss4;
  ss4 << rate;
  std::string sRate = ss4.str ();

  NS_LOG_INFO ("Configure Tracing.");
  return tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + sNodeSpeed + "speed_" + sNodePause + "pause_" + sRate + "rate";
}

void
RoutingExperiment::SetupFlowmon ()
{
  m_flowmon = m_flowmonHelper.Install (m_allNodes);
}

// HistHelper ----------------------------

HistHelper::HistHelper (Histogram &hist)
  : m_histogram(hist)
{
}

double
HistHelper::Average ()
{
  if (m_histogram.GetNBins () == 0) {
    return 0;
  }

  double total = 0;
  for (uint32_t i = 0; i < m_histogram.GetNBins (); i++)
  {
    total += static_cast<double> (m_histogram.GetBinCount (i)) * ( static_cast<double> (m_histogram.GetBinStart (i)) + ( static_cast<double> (m_histogram.GetBinWidth (i)) / 2.0));
  }

  return total / static_cast<double> (m_histogram.GetNBins ());
}

std::ostream& operator <<(std::ostream& os, const Result& result)
{
  return os << "Result { protocolName: "
            << result.protocolName
            << ", nNodes: "
            << result.nNodes
            << ", averageDelay: "
            << result.averageDelay
            << ", averageJitter: "
            << result.averageJitter
            << ", packetDeliveryRatio: "
            << result.packetDeliveryRatio
            << "\%, throughput: "
            << result.throughput
            << "kbps, packetOverhead: "
            << result.packetOverhead
            << ", byteOverhead: "
            << result.byteOverhead
            << " }";
}

// Scenario ------------------------------

Scenario::Scenario (int nNodes, int size)
  : nNodes (nNodes),
    size (size)
{
}
