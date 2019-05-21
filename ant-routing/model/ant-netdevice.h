#ifndef ANT_NETDEVICE_H
#define ANT_NETDEVICE_H

#include "ant-queue-entry.h"
#include <memory>
#include <queue>

namespace ns3 {
namespace ant_routing {

class AntNetDevice {
public:
  AntNetDevice();
  AntNetDevice(Ptr<NetDevice> device);

 ~AntNetDevice();

  Ptr<NetDevice> GetDevice();
  void SetDevice(Ptr<NetDevice> device);

  void Send(AntQueueEntry entry);
  void SendExpedited(AntQueueEntry entry);

  // returns the size of the std queue (not used to expedite ants)
  std::size_t QueueSize();

  // moving average of the send time of the node
  Time SendTimeEstimate();

  static double GetAlpha();
  static void SetAlpha(double alpha);

private:

  // callback to be called when the transmission failed
  void DroppedPacketCallback(Ptr<const Packet> packet);

  // callback to be called when the transmission was successful
  void DeliveredPacketCallback(Ptr<const Packet> packet);

  // the alpha value for calculating the average send time
  static double s_alpha;
  // Pimpl
  struct AntNetDeviceImpl;
  // pointer to implementation
  std::shared_ptr<AntNetDeviceImpl> m_impl;
};

} // namespace ant_routing
} // namespace ns3

#endif // ANT_NETDEVICE_H
