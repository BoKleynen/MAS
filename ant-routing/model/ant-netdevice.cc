#include "ant-netdevice.h"

namespace ns3 {
  NS_LOG_COMPONENT_DEFINE("AntNetDevice");

namespace ant_routing {

// AntNetDeviceImpl definition -------------------------------------------------

struct AntNetDevice::AntNetDeviceImpl {
  // type definitions :

  using SendQueue = std::queue<std::shared_ptr<SendQueueEntry>>;

  // constructor & destructor

  AntNetDeviceImpl();
  explicit AntNetDeviceImpl(Ptr<NetDevice> device);

  ~AntNetDeviceImpl();

  // methods

  //check if the underlying device is idle (no entries in the queue)
  bool IsIdle();

  void SubmitTo(std::shared_ptr<SendQueueEntry> entry, SendQueue& queue);

  // sends the next packet. Note that this operation does not
  // remove an entry from the queue as the entry is used to record the sending
  // time of the packet. The entry is removed only after ack or error
  void SendNext();

  // caputerer for drops at the mac layer
  void MacTxDropCallback(Ptr<const Packet> p);

  // capturer for failure of sending
  void TxErrHeaderCallback(const WifiMacHeader& h);

  // capturer for successful sending (returns afer unsuccesful sending)
  void TxOkHeaderCallback(const WifiMacHeader& h);

  // callback to be called when the transmission failed
  void DroppedPacketCallback();

  // callback to be called when the transmission was successful
  void DeliveredPacketCallback();

  // hooks up the trace sources from the NetDevice to the AntNetDevice
  void HookupTraces(Ptr<NetDevice> device);

  // unhooks the trace sources attached to the device. Is used when
  // the AntNetDeviceImpl is destructed
  void UnhookTraces(Ptr<NetDevice> device);

  // members:

  Ptr<NetDevice> m_device;
  SendQueue m_stdQueue;
  SendQueue m_fastQueue;
  Time m_sendTimeEst; // estimate of the time needed to send a message over the channel
  std::size_t m_maxQueueSize; // the max queue size for each queue before we start dropping packets
  bool m_tracesHooked;
};

AntNetDevice::AntNetDeviceImpl::AntNetDeviceImpl()
  : AntNetDeviceImpl(Ptr<NetDevice>()) { }


// note: we call the hookup in the constructor of the AntNetDeviceImpl instead
// of the constructor of the AntNetDevice since the latter is a reference type
// and the 'underlying' device must only be hooked up once
AntNetDevice::AntNetDeviceImpl::AntNetDeviceImpl(Ptr<NetDevice> device)
  : m_device(device), m_stdQueue(SendQueue()), m_fastQueue(SendQueue()), m_maxQueueSize(DEFAULT_MAX_QUEUESIZE), m_tracesHooked(false) {
    HookupTraces(device);
  }

AntNetDevice::AntNetDeviceImpl::~AntNetDeviceImpl() {
  UnhookTraces(m_device);
}

bool
AntNetDevice::AntNetDeviceImpl::IsIdle() {
  return m_fastQueue.empty() && m_stdQueue.empty();
}

void
AntNetDevice::AntNetDeviceImpl::SubmitTo(std::shared_ptr<SendQueueEntry> entry, SendQueue& queue) {
  bool idle = IsIdle();
  queue.push(entry);
  if(idle) {
    SendNext();
  }
}


void
AntNetDevice::AntNetDeviceImpl::SendNext() {
  // send function (to spare some lines of code -> issues with references)

  auto send = [] (auto nxt) {
    nxt->Sending(true);
    nxt->SendStartTime(Simulator::Now());
    (*nxt)();
  };

  if(!m_fastQueue.empty()) {
    auto nxt = m_fastQueue.front();
    send(nxt);
    return;
  }

  if(!m_stdQueue.empty()) {
    auto nxt = m_stdQueue.front();
    send(nxt);
    return;
  }

  // do nothing if no pending packets available.
}

void
AntNetDevice::AntNetDeviceImpl::HookupTraces(Ptr<NetDevice> device) {

  // if the traces are already hooked or the device is a nullpointer do not hookup the traces
  if (m_tracesHooked || !device) {
    return;
  }

  // hookup the delivery callbacks.
  // note: since we do spoon feed the mac layer, we need no callbacks for when
  // the mac layer is starting the transaction.
  Ptr<WifiNetDevice> wifiDev = device -> GetObject<WifiNetDevice>();
  Ptr<WifiMac> wifiMac = wifiDev -> GetMac();
  Ptr<RegularWifiMac> regWifiMac = wifiMac -> GetObject<RegularWifiMac>();

  wifiMac -> TraceConnectWithoutContext(MacTxDrop, MakeCallback(&AntNetDevice::AntNetDeviceImpl::MacTxDropCallback, this));
  regWifiMac -> TraceConnectWithoutContext(TxErrHeader, MakeCallback(&AntNetDevice::AntNetDeviceImpl::TxErrHeaderCallback, this));
  regWifiMac -> TraceConnectWithoutContext(TxOkHeader, MakeCallback(&AntNetDevice::AntNetDeviceImpl::TxOkHeaderCallback, this));

  m_tracesHooked = true;
}

void
AntNetDevice::AntNetDeviceImpl::UnhookTraces(Ptr<NetDevice> device) {
  // if there is no device hooked up, no reason to un-hook it;
  if(!m_tracesHooked) {
    return;
  }

  Ptr<WifiNetDevice> wifiDev = device -> GetObject<WifiNetDevice>();
  Ptr<WifiMac> wifiMac = wifiDev -> GetMac();
  if(!wifiMac) {
    return; // distructor issue with the Ptr<> type and the simulator;
  }
  Ptr<RegularWifiMac> regWifiMac = wifiMac -> GetObject<RegularWifiMac>();


  wifiMac -> TraceDisconnectWithoutContext(MacTxDrop, MakeCallback(&AntNetDevice::AntNetDeviceImpl::MacTxDropCallback, this));
  regWifiMac -> TraceDisconnectWithoutContext(TxErrHeader, MakeCallback(&AntNetDevice::AntNetDeviceImpl::TxErrHeaderCallback, this));
  regWifiMac -> TraceDisconnectWithoutContext(TxOkHeader, MakeCallback(&AntNetDevice::AntNetDeviceImpl::TxOkHeaderCallback, this));
}

// caputerer for drops at the mac layer
void
AntNetDevice::AntNetDeviceImpl::MacTxDropCallback(Ptr<const Packet> p) {
  DroppedPacketCallback();
}

// capturer for failure of sending
void
AntNetDevice::AntNetDeviceImpl::TxErrHeaderCallback(const WifiMacHeader& h) {
  DroppedPacketCallback();
}

// capturer for successful sending (returns afer unsuccesful sending)
void
AntNetDevice::AntNetDeviceImpl::TxOkHeaderCallback(const WifiMacHeader& h) {
  DeliveredPacketCallback();
}


// callback to be called when the transmission failed
void
AntNetDevice::AntNetDeviceImpl::DroppedPacketCallback() {
  if(!m_fastQueue.empty() && m_fastQueue.front()->Sending()) {
    m_fastQueue.pop();
    SendNext();
  }

  if(!m_stdQueue.empty() && m_stdQueue.front()->Sending()) {
    m_stdQueue.pop();
    SendNext();
  }

  //NS_LOG_WARN("Warning: " << m_device -> GetAddress() << " receive dropped callback without any sending entries in the standard or fast queue");
}

// callback to be called when the transmission was successful
void
AntNetDevice::AntNetDeviceImpl::DeliveredPacketCallback() {

  auto handleSent = [this] (SendQueue& queue) {
    auto elapsedTime = Simulator::Now() - queue.front()->SendStartTime();
    m_sendTimeEst = Seconds(s_alpha * m_sendTimeEst.GetSeconds() + (1 - s_alpha) * elapsedTime.GetSeconds());
    queue.pop();
    SendNext();
  };

  if(!m_fastQueue.empty() && m_fastQueue.front()->Sending()) {
    handleSent(m_fastQueue);
    return;
  }

  if(!m_stdQueue.empty() && m_stdQueue.front()->Sending()) {
    handleSent(m_stdQueue);
    return;
  }

  //NS_LOG_WARN("Warning: " << m_device -> GetAddress() << "received 'DeliveredPacketCallback' without any sending entries in the standard or fast queue");
}

// AntNetDevice definition -----------------------------------------------------
// static constants ------------------------------------------------------------
// constexpr std::string AntNetDevice::MacTxDrop = "MacTxDrop";
// constexpr std::string AntNetDevice::TxOkHeader = "TxOkHeader";
// constexpr std::string AntNetDevice::TxErrHeader = "TxErrHeader";
// static variable definition --------------------------------------------------
double AntNetDevice::s_alpha = 0.5;

// method definition -----------------------------------------------------------
AntNetDevice::AntNetDevice() : AntNetDevice(Ptr<NetDevice>()) { }

AntNetDevice::AntNetDevice(Ptr<NetDevice> device) : m_impl(std::make_shared<AntNetDeviceImpl>(device)) { }

AntNetDevice::~AntNetDevice() { }

Ptr<NetDevice>
AntNetDevice::Device() {
  return m_impl -> m_device;
}

void
AntNetDevice::Device(Ptr<NetDevice> device) {
  // the device changed, first unhook the traces
  m_impl -> UnhookTraces(m_impl -> m_device);
  // then set the new device
  m_impl -> m_device = device;
  // hook up the traces again
  m_impl -> HookupTraces(device);
}

void
AntNetDevice::Submit(std::shared_ptr<SendQueueEntry> entry) {
  if(m_impl -> m_stdQueue.size() > MaxQueueSize()) {
    return; // drop the packet. TODO do we add a trace source for this?
  }

  NS_LOG_UNCOND("Submitted normal packet");

  m_impl -> SubmitTo(entry, m_impl -> m_stdQueue);
}

void
AntNetDevice::SubmitExpedited(std::shared_ptr<SendQueueEntry> entry) {
  if(m_impl -> m_fastQueue.size() > MaxQueueSize()) {
    return;
  }

  NS_LOG_UNCOND("Submitted expedited entry, queue size: " << m_impl -> m_fastQueue.size());

  m_impl -> SubmitTo(entry, m_impl -> m_fastQueue);
}

// returns the size of the std queue (not used to expedite ants)
std::size_t
AntNetDevice::QueueSize() {
  return m_impl -> m_stdQueue.size();
}

// moving average of the send time of the node
Time
AntNetDevice::SendingTimeEst() {
  return m_impl -> m_sendTimeEst;
}

std::size_t
AntNetDevice::MaxQueueSize() {
  return m_impl -> m_maxQueueSize;
}

void
AntNetDevice::MaxQueueSize(std::size_t maxQueueSize) {
  m_impl -> m_maxQueueSize = maxQueueSize;
}

double
AntNetDevice::GetAlpha() {
  return s_alpha;
}

void
AntNetDevice::SetAlpha(double alpha) {
  s_alpha = alpha;
}

ExpeditedTag::ExpeditedTag(uint8_t expedited)
  : m_expedited(expedited) {

  }
bool ExpeditedTag::IsExpedited() const {
  return m_expedited != 0;
}

TypeId ExpeditedTag::GetTypeId() {
  static TypeId tid = TypeId ("ns3::ant_routing::ExpeditedTag")
    .SetParent<Tag> ()
    .SetGroupName ("AntRouting")
    .AddConstructor<ExpeditedTag> ()
  ;
  return tid;
}
TypeId ExpeditedTag::GetInstanceTypeId() const {
  return GetTypeId();
}
uint32_t ExpeditedTag::GetSerializedSize() const {
  return sizeof(uint8_t);
}
void ExpeditedTag::Serialize(TagBuffer i) const {
  i.WriteU8(m_expedited);
}

void ExpeditedTag::Deserialize(TagBuffer i) {
  m_expedited = i.ReadU8();
}
void ExpeditedTag::Print(std::ostream &os) const {
  os << "Do don't do that around here";
}

} // namespace ant_routing
} // namespace ns3
