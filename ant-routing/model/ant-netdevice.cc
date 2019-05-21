#include "ant-netdevice.h"

namespace ns3 {
namespace ant_routing {

// AntNetDeviceImpl definition -------------------------------------------------

struct AntNetDeviceImpl {

  AntNetDeviceImpl();
  AntNetDeviceImpl(Ptr<NetDevice> device);

  using AntQueue = std::queue<AntQueueEntry>;

  Ptr<NetDevice> m_netDevice;
  std::shared_ptr<AntQueue> m_stdQueue;
  std::shared_ptr<AntQueue> m_fastQueue;
  Time m_stdQueueTime;
}
AntNetDevice::AntNetDeviceImpl::AntNetDeviceImpl()
  : AntNetDeviceImpl(Ptr<NetDevice> device);

AntNetDevice::AntNetDeviceImpl::AntNetDeviceImpl(Ptr<NetDevice> device)
  : m_netDevice(device), m_stdQueue(AntQueue()), m_fastQueue(AntQueue()) { }

// AntNetDevice definition -----------------------------------------------------
// static variable definition --------------------------------------------------
double AntNetDevice::s_alpha = 0.5

// method definition -----------------------------------------------------------
AntNetDevice::AntNetDevice() : AntNetDevice(Ptr<NetDevice>());

AntNetDevice::AntNetDevice(Ptr<NetDevice> device) :  {
  // TODO hook up the traced callbacks for failure and success
}

AntNetDevice::~AntNetDevice() { }

Ptr<NetDevice> AntNetDevice::GetDevice() {
  return m_impl -> m_device;
}
void AntNetDevice::SetDevice(Ptr<NetDevice> device) {
  m_impl -> m_device;
}

void AntNetDevice::Send(AntQueueEntry entry) {
  m_impl -> m_stdQueue.push(entry);
}

void AntnetDevice::SendExpedited(AntQueueEntry entry) {
  m_impl -> m_fastQueue.push(entry);
}

// returns the size of the std queue (not used to expedite ants)
std::size_t AntNetDevice::QueueSize() {
  return m_impl -> m_stdQueue;
}

// moving average of the send time of the node
Time AntNetDevice::SendTimeEstimate() {
  return m_impl -> m_stdQueueTime;
}

double AntNetDevice::GetAlpha() {
  return s_alpha;
}
void AntNetDevice::SetAlpha(double alpha) {
  s_alpha = alpha;
}

// callback to be called when the transmission failed
void AntNetDevice::DroppedPacketCallback(Ptr<const Packet> packet) {
  // TODO implement the failure of delivery callback,must do an ErrorCallback
}

// callback to be called when the transmission was successful
void AntNetDevice::DeliveredPacketCallback(Ptr<const Packet> packet) {
  // TODO implement the delivery callback, we must do a ucb callback
}

} // namespace ant_routing
} // namespace ns3
