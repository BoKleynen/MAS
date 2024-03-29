#ifndef ANT_NETDEVICE_H
#define ANT_NETDEVICE_H

#include "send-queue-entry.h"
#include "ns3/wifi-module.h"
#include "ns3/packet.h"
#include <memory>
#include <queue>

namespace ns3 {
namespace ant_routing {

// callback function, used to notify the interested parties that sending
// a certain packet has failed at the mac layer
using RouteRepairCallback = std::function<std::shared_ptr<SendQueueEntry>(Ipv4Address, Ipv4Address)>;

class AntNetDevice {
public:
  AntNetDevice();
  AntNetDevice(Ptr<NetDevice> device);

 ~AntNetDevice();

  Ptr<NetDevice> Device();
  void Device(Ptr<NetDevice> device);

  void Submit(std::shared_ptr<SendQueueEntry> entry);
  void SubmitExpedited(std::shared_ptr<SendQueueEntry> entry);

  // returns the size of the std queue (not used to expedite ants)
  std::size_t QueueSize();

  // moving average of the send time of the node
  Time SendingTimeEst();
  //
  static std::size_t MaxQueueSize();
  static void MaxQueueSize(std::size_t size);

  void SetRouteRepairCallback(RouteRepairCallback cb);

  static double GetAlpha();
  static void SetAlpha(double alpha);

  static bool GetRepairEnabled();
  static void SetRepairEnabled(bool enabled);

private:
  // the alpha value for calculating the average send time
  static double s_alpha;
  static std::size_t s_maxQueueSize;
  static bool s_repairEnabled;

  static constexpr const char* MacTxDrop = "MacTxDrop";
  static constexpr const char* TxOkHeader = "TxOkHeader";
  static constexpr const char* TxErrHeader = "TxErrHeader";

  static constexpr std::size_t DEFAULT_MAX_QUEUESIZE = 20;

  // Pimpl
  struct AntNetDeviceImpl;
  // pointer to implementation
  std::shared_ptr<AntNetDeviceImpl> m_impl;
};

class ExpeditedTag : public Tag {
public:
  ExpeditedTag(uint8_t expedited = 1);
  bool IsExpedited() const;
  static TypeId GetTypeId();
  virtual TypeId GetInstanceTypeId () const override;
  virtual uint32_t GetSerializedSize() const override;
  virtual void Serialize(TagBuffer i) const override;
  virtual void Deserialize(TagBuffer i) override;
  virtual void Print(std::ostream &os) const override;
private:
  // expeditedif m_expedited != 0
  uint8_t m_expedited;
};

} // namespace ant_routing
} // namespace ns3

#endif // ANT_NETDEVICE_H
