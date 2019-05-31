#include "manet-routing-compare.h"
#include "param-experiment-suite.h"
using namespace ns3;

// Rho experiment --------------------------------------------------------------

void RhoSetter(double val) {
  ant_routing::AntRoutingTable::Rho(val);
}

void RhoPrinter(double val, ant_experiment::Result result) {
  std::cout << "Rho: " << val << " - " << result << std::endl;
}

const std::vector<double> rhoVector {0.0, 0.25, 0.50, 0.75, 1.0 };

// Ant activation experiment ---------------------------------------------------
void AntActivationSetter(std::pair<bool, bool> val) {
  ant_routing::AntNetDevice::SetRepairEnabled(val.first);
  ant_routing::AnthocnetRouting::SetProactiveEnabled(val.second);
}

void AntActivationPrinter(std::pair<bool, bool> val, ant_experiment::Result result) {
  std::cout << std::boolalpha;
  std::cout << "Repair enabled: " << val.first << "; Proactive enabled: " << val.second << " - " << result << std::endl;
  std::cout << std::noboolalpha;
}

const std::vector<std::pair<bool, bool>> comboVector {
  std::pair<bool, bool> { true,  true  },
  std::pair<bool, bool> { true,  false },
  std::pair<bool, bool> { false, true  },
  std::pair<bool, bool> { false, false }
  };

// update on failure -----------------------------------------------------------
void UpdateOnFailureSetter(bool val) {
  ant_routing::LinkFailureAnt::PheromoneUpdatesOnFailureEnabled(val);
}

void UpdateOnFailurePrinter(bool val, ant_experiment::Result result) {
  std::cout << std::boolalpha;
  std::cout << "Pheromone update on failure enabled: " << val << " - " << result << std::endl;
  std::cout << std::noboolalpha;
}

const std::vector<bool> onOffVector { true, false };


// PacketBeta experiment -------------------------------------------------------
void PacketBetaSetter(double val) {
  ant_routing::AntRoutingTable::PacketBeta(val);
}

void PacketBetaPrinter(double val, ant_experiment::Result result ) {
  std::cout << "Packet beta: " << val << " - " << result << std::endl;
}

const std::vector<double> betaVector { 0.5, 1.0, 1.5, 2.0, 2.5, 3.0 };


// Ant beta experiment ---------------------------------------------------------

void AntBetaSetter(double val) {
  ant_routing::AntRoutingTable::AntBeta(val);
}

void AntBetaPrinter(double val, ant_experiment::Result result ) {
  std::cout << "ant beta: " << val << " - " << result << std::endl;
}


// Acceptance factor -----------------------------------------------------------

void AdmissionFactorSetter(double val) {
  ant_routing::ReactiveQueen::AdmissionRatio(val);
  ant_routing::RepairQueen::AdmissionRatio(val);
}

void AdmissionFactorPrinter(double val, ant_experiment::Result result) {
  std::cout << "Admission factor: " << val << " - " <<  result << std::endl;
}

const std::vector<double> admissionVector { 0.5, 1.0, 1.5, 2.0, 2.5 };

// proactive probability -------------------------------------------------------
void ProbabilitySetter(double val) {
  ant_routing::AnthocnetRouting::SetProactiveProbability(val);
}


void ProbabilityPrinter(double val, ant_experiment::Result result) {
  std::cout << "Proactive probability: " << val << " - " << result << std::endl;
}

const std::vector<double> probabilityVector { 0.05, 0.1, 0.15, 0.20 };

// -----------------------------------------------------------------------------

int
main (int argc, char *argv[])
{

  std::cout << "Running comparison experiment: Anthocnet Vs Aodv" << std::endl;
  compare_experiments::RunComparisonExperiment();

  std::cout << "Running the Rho tweaking experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite rhoExperiment;
  rhoExperiment.RunSuite<double>(rhoVector, RhoSetter, RhoPrinter);

  std::cout << "Running the ant activation experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite antActivationExperiment;
  antActivationExperiment.RunSuite<std::pair<bool, bool>>(comboVector, AntActivationSetter, AntActivationPrinter);

  std::cout << "Running the update pheromone on failure enabled experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite failurePheromoneSuite;
  failurePheromoneSuite.RunSuite<bool>(onOffVector, UpdateOnFailureSetter, UpdateOnFailurePrinter);

  std::cout << "Running the packet beta experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite betaPacketExperiment;
  betaPacketExperiment.RunSuite<double>(betaVector, PacketBetaSetter, PacketBetaPrinter);

  std::cout << "Running the ant beta experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite betaAntExperiment;
  betaAntExperiment.RunSuite<double>(betaVector, AntBetaSetter, AntBetaPrinter);

  std::cout << "Running the admission factor experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite admissionFactorExperiment;
  admissionFactorExperiment.RunSuite<double>(admissionVector, AdmissionFactorSetter, AdmissionFactorPrinter);

  std::cout << "Running the proactive ant probability experiment: " << std::endl;
  ant_experiment::ParamExperimentSuite probabilityExperiment;
  probabilityExperiment.RunSuite<double>(probabilityVector, ProbabilitySetter, ProbabilityPrinter);

}
