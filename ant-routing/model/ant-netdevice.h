#ifndef ANT_NETDEVICE_H
#define ANT_NETDEVICE_H

#include "ant-queue-entry.h"
#include "ns3/wifi-module.h"
#include "ns3/packet.h"
#include <memory>
#include <queue>

namespace ns3 {
namespace ant_routing {

class AntNetDevice {
public:
  AntNetDevice();
  AntNetDevice(Ptr<NetDevice> device);

 ~AntNetDevice();

  Ptr<NetDevice> Device();
  void Device(Ptr<NetDevice> device);

  void Submit(const AntQueueEntry& entry);
  void SubmitExpedited(const AntQueueEntry& entry);

  // returns the size of the std queue (not used to expedite ants)
  std::size_t QueueSize();

  // moving average of the send time of the node
  Time SendingTimeEst();

  std::size_t MaxQueueSize();
  void MaxQueueSize(std::size_t size);

  static double GetAlpha();
  static void SetAlpha(double alpha);

private:
  // the alpha value for calculating the average send time
  static double s_alpha;

  static constexpr const char* MacTxDrop = "MacTxDrop";
  static constexpr const char* TxOkHeader = "TxOkHeader";
  static constexpr const char* TxErrHeader = "TxErrHeader";

  static constexpr std::size_t DEFAULT_MAX_QUEUESIZE = 20;

  // Pimpl
  struct AntNetDeviceImpl;
  // pointer to implementation
  std::shared_ptr<AntNetDeviceImpl> m_impl;
};

} // namespace ant_routing
} // namespace ns3

#endif // ANT_NETDEVICE_H
