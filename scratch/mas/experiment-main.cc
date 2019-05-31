// #include "rho-compare-experiment.h"
#include "param-experiment-suite.h"
using namespace ns3;

// Rho experiment ---------------------------

void RhoSetter(double val) {
  std::cout << "value: " << val << std::endl;
  ant_routing::AntRoutingTable::Rho(val);
}

void RhoPrinter(double val, ant_experiment::Result result) {
  std::cout << "Rho: " << val << " - " << result << std::endl;
}

const std::vector<double> rhoVector {0.0, 0.25, 0.50, 0.75, 1.0 };

// Repair Experiment -----------------------------------------------
void RepairSetter(bool val) {
  ant_routing::AntNetDevice::SetRepairEnabled(val);
}

void RepairPrinter(bool val, ant_experiment::Result result) {
  std::cout << std::boolalpha;
  std::cout << "Repair ant enabled: " << val << " - " << result << std::endl;
  std::cout << std::noboolalpha;
}

const std::vector<bool> switchVector {true, false};

// Proactive Experiment --------------------------------------------
void ProactiveSetter(bool val) {
  ant_routing::AnthocnetRouting::SetProactiveEnabled(val);
}

void ProactivePrinter(bool val, ant_experiment::Result result) {
  std::cout << std::boolalpha;
  std::cout << "Proactive ant disabled: " << val << " - " << result << std::endl;
  std::cout << std::noboolalpha;
}

// ------------------------------------------------------------------

int
main (int argc, char *argv[])
{
  NS_LOG_UNCOND("Starting manet routing compare");
  // RhoExperimentSuite experimentSuite (5);
  // experimentSuite.RunSuite ();
  // ParamExperimentSuite rhoExperiment;
  // rhoExperiment.RunSuite<double>(rhoVector, RhoSetter, RhoPrinter);

  // ant_experiment::ParamExperimentSuite repairExperiment;
  // repairExperiment.RunSuite<bool>(switchVector, RepairSetter, RepairPrinter);

  ant_experiment::ParamExperimentSuite proactiveExperiment;
  proactiveExperiment.RunSuite<bool>(switchVector, ProactiveSetter, ProactivePrinter);

}
