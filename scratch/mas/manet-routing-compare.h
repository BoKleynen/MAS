#include <fstream>
#include <iostream>
#include <algorithm>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ant-routing-module.h"

#ifndef ROUTING_EXPERIMENT
#define ROUTING_EXPERIMENT

using namespace ns3;

struct Scenario {
  int nNodes;
  int size;

  Scenario (int nNodes, int size);
  Scenario(const Scenario& scenario) = default;
  Scenario& operator=(const Scenario& scenario) = default;

};

struct Result
{
    std::string protocolName;
    uint64_t nNodes;
    double averageDelay;
    double averageJitter;
    double packetDeliveryRatio;
    double throughput; // in kbps
    double packetOverhead;
    double byteOverhead;
};

class RoutingExperimentSuite
{
public:
  RoutingExperimentSuite (uint8_t nSimulations);

  void RunSuite (void);
  std::vector<Result> GetResult () const;
private:
  uint8_t m_nSimulations;
  std::vector<Result> m_results;
  std::vector<Scenario> m_scenarios;
};

class HistHelper
{
public:
    HistHelper (Histogram &hist);
    double Average ();

private:
    Histogram &m_histogram;

};

class RoutingExperiment
{
public:
  // RoutingExperiment ();
  RoutingExperiment (uint32_t protocol, int nSinks, Scenario scenario);
  void Run ();
  // static void SetMACParam (ns3::NetDeviceContainer & devices,
  //                                 int slotDistance);
  // std::string CommandSetup (int argc, char **argv);
  Result GetResult (void);

private:
  static constexpr const char* phyMode = "DsssRate11Mbps";
  static constexpr const char* rate = "2048bps";

  Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
  // void CheckThroughput ();

  void SetupNodes ();
  void SetupWifi (void);
  void SetupMobility (void);
  void SetupProtocol (void);
  void SetupAddresses (void);
  void SetupApplications (void);
  void SetupFlowmon (void);
  std::string TraceName (void);

  uint32_t port;
  uint32_t bytesTotal;
  uint32_t packetsReceived;

  bool m_traceMobility;
  uint32_t m_protocol;
  double m_txp;
  int m_nSinks;
  Scenario m_scenario;

  double TotalTime = 200.0;
  std::string m_protocolName;

  int nodePause = 20;
  int nodeSpeed = 5;

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  NodeContainer m_allNodes;
  NodeContainer m_adhocNodes;
  NodeContainer m_senderNodes;
  NodeContainer m_receiverNodes;

  NetDeviceContainer m_allDevices;
  NetDeviceContainer m_adhocDevices;
  NetDeviceContainer m_senderDevices;
  NetDeviceContainer m_receiverDevices;

  Ipv4InterfaceContainer m_adhocInterfaces;
  Ipv4InterfaceContainer m_senderInterfaces;
  Ipv4InterfaceContainer m_receiverInterfaces;

  Ptr<FlowMonitor> m_flowmon;
  FlowMonitorHelper m_flowmonHelper;
};

std::ostream& operator <<(std::ostream& os, const Result& result);

#endif // ROUTING_EXPERIMENT
