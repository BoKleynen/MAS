#include "ant-hill.h"
#include "ant.h"
#include <map>

namespace ns3 {
namespace ant_routing {

struct AntHill::AntHillImpl {
  using QueenMap = std::map<AntType, std::shared_ptr<AntQueen>>;

  AntHillImpl();
  QueenMap m_queens;
};

AntHill::AntHillImpl::AntHillImpl() : m_queens(QueenMap()) { }

AntHill::AntHill() : m_impl(std::make_shared<AntHillImpl>()) {}

AntHill::~AntHill() {}

std::shared_ptr<Ant>
AntHill::CreateFrom(const AntHeader& header) {
  auto queen  = m_impl -> m_queens.find(header.GetAntType());
  if (queen == m_impl -> m_queens.end()) {
    return nullptr;
  }

  return queen->second->CreateFrom(header);
}


void
AntHill::AddQueen(std::shared_ptr<AntQueen> queen) {
  m_impl -> m_queens[queen->GetAntType()] = queen;
}

std::shared_ptr<AntQueen>
AntHill::GetQueen(AntType antType) {
  auto queen = m_impl -> m_queens.find(antType);
  return queen != m_impl -> m_queens.end() ? queen -> second : nullptr;
}


} // namespace ant_routing
} // namespace ns3
