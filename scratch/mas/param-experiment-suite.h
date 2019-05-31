#ifndef PARAM_EXPERIMENT_SUITE
#define PARAM_EXPERIMENT_SUITE

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
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
// #include "manet-routing-compare.h"

namespace ns3 {
namespace ant_experiment {

struct Scenario {
  int nNodes;
  int size;

  Scenario (int nNodes, int size);
  Scenario(const Scenario& scenario) = default;
  Scenario& operator=(const Scenario& scenario) = default;

};

struct Result
{
    Result();
    std::string protocolName;
    uint64_t nNodes;
    double averageDelay;
    double averageJitter;
    double packetDeliveryRatio;
    double throughput; // in kbps
    double packetOverhead;
    double byteOverhead;
};

std::ostream& operator <<(std::ostream& os, const Result& result);


Result AverageResult(std::vector<Result> results);

class HistHelper
{
public:
    HistHelper (Histogram &hist);
    double Average ();

private:
    Histogram &m_histogram;

};


class ParamExperiment
{
public:
  // RoutingExperiment ();
  ParamExperiment (uint32_t protocol, int nSinks, Scenario scenario);
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

class ParamExperimentSuite
{
public:
  ParamExperimentSuite (uint8_t nSimulations = 5);

  // void RunSuite (void);
  std::vector<Result> GetResult () const;

  template<typename T>
  void RunSuite(std::vector<T> paramValues, std::function<void(T)> paramSetter ,std::function<void(T, Result)> printer);
private:
  uint8_t m_nSimulations;
  std::vector<Result> m_results;
  std::vector<Scenario> m_scenarios;
};
template<typename T>
void ParamExperimentSuite::RunSuite(std::vector<T> paramValues,
                                    std::function<void(T)> paramSetter,
                                    std::function<void(T, Result)> printer) {

  int nSinks = 5;
  for (auto scenario : m_scenarios)
  {
    for (auto value : paramValues)
    {
      std::vector<Result> expResults;
      paramSetter(value);
      for (int i = 0; i < m_nSimulations; i++)
      {
        ParamExperiment experiment (4, nSinks, scenario);
        RngSeedManager::SetRun(i+1);
        experiment.Run ();
        expResults.push_back(experiment.GetResult());
      }
      auto avgResult = AverageResult(expResults);
      m_results.push_back (avgResult);
      printer(value, avgResult);
    }
  }
}

} // namespace ant_experiment
} // namespace ns3

#endif
