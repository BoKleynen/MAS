#ifndef ANT_NEIGHBOR_H
#define ANT_NEIGHBOR_H

#include "ant-netdevice.h"
#include "ant-packet.h"

#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <memory>
#include <iostream>
#include <functional>

namespace ns3 {
namespace ant_routing {

class NeighborFailureDetector;

// Class representing a neighbor node
class Neighbor {
public:

  friend bool operator<(const Neighbor& lhs, const Neighbor& rhs);
  friend class AntRoutingTable;
  // todo add a default device as a static variable? allows to add based on address only
  Neighbor();
  Neighbor(Ipv4Address addr, AntNetDevice device);

  ~Neighbor();

  // creates a route fronm souce to destination based on the configuration
  // of the neighbor
  Ptr<Ipv4Route> CreateRoute(Ipv4Address source, Ipv4Address destination);

  Ipv4Address Address() const;
  void Address(Ipv4Address addr);

  AntNetDevice AntDevice();
  void AntDevice(AntNetDevice device);

  // submits a packet to the destination, placing the packet in the sending queue
  // param route:     Route containing next hop to destination
  // param packet:    the packet to deliver
  // param header:    Ipv4 header containing information about the packet for the network layer
  // param callback:  Callback function to be called once the packet is to be sumbitted
  void SubmitPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet, const Ipv4Header &header, UnicastCallback callback);
  void SubmitPacket(Ptr<const Packet> packet, const Ipv4Header& header, UnicastCallback callback);
  // submits a packet to the destination, using the expedited lane. (used for ants for example)
  void SubmitExpeditedPacket(Ptr<Ipv4Route> route, Ptr<const Packet> packet, const Ipv4Header &header, UnicastCallback callback);
  void SubmitExpeditedPacket(Ptr<const Packet> packet, const Ipv4Header& header, UnicastCallback callback);
  // template<typename SendQueueType, typename ...Args>
  // void Submit(Args...args) {
  //   auto queueEntry = std::make_shared<SendQueueType>(std::forward<Args>(args)...);
  //   auto dev = AntDevice();
  //   dev.Submit(queueEntry);
  // }
  //
  // template<typename SendQueueType, typename ...Args>
  // void SubmitExpedited(Args...args) {
  //   auto queueEntry = std::make_shared<SendQueueType>(std::forward<Args>(args)...);
  //   auto dev = AntDevice();
  //   dev.SubmitExpedited(queueEntry);
  // }

  std::shared_ptr<NeighborFailureDetector> FailureDetector();
  void FailureDetector(std::shared_ptr<NeighborFailureDetector> detector);

private:
  Neighbor(std::shard_ptr<NeighborImpl> impl);
  struct NeighborImpl;

  const std::shared_ptr<NeighborImpl> Data() const;
  std::shared_ptr<NeighborImpl> Data();

  void Data(std::shared_ptr<NeighborImpl> impl);
  // pointer that holds the data for the neighbor
  std::shared_ptr<NeighborImpl> m_impl;
};

// inequality operator, allows the DefaultNeighbor type to be a key of a
// std::map
bool operator<(const Neighbor& lhs, const Neighbor& rhs);
bool operator==(const Neighbor& lhs, const Neighbor& rhs);
std::ostream& operator<<(std::ostream& os, const Neighbor& nb);

// base class for neighbor failure detection, outsources the fialure
// detection mechanism of the neighbor.
class NeighborFailureDetector {
public:
  // typedef of a function type that is called when failure of the neighbor
  // occurs
  using FailureCallback = std::function<void(Neighbor neighbor)>;

  // creates a failure detector
  // after the detector is created call "Start()" on the detector
  // or use the `MakeFailureDetector` function to do this safely
  NeighborFailureDetector(Neighbor neighbor);
  virtual ~NeighborFailureDetector();

  // function to be implemented by the implementor.
  // can update the appropriate data (structures) inside the failure handler
  virtual void HelloReceived(const HelloHeader& header)= 0;

  // registers a callback. This function will be called when failure of a
  // neighbor is detected
  void RegisterFailureCallback(FailureCallback func);

  // clears all the registered callbacks;
  void ClearAllCallbacks();

  // to be called after the creation of the application, first calls the OnStart hook
  // then does other work.
  void Start();

  // suspend the failure checker (do not do regular checkups);
  void Suspend();

  // resume the failure checked (continue doing regular checkups);
  void Resume();

  bool IsSuspended();

protected:
  // called at regular intervals, configrable via the 'CheckInterval' function
  virtual bool HasFailed() = 0;
  // getting & setting the interval on which to call next
  Time CheckInterval();
  void CheckInterval(Time checkInterval);

  // callback to the failure checker when it gets started for the first time
  virtual void OnStart();
  // callback to the failure checker when it is suspended, may want to do some cleanup
  virtual void OnSuspend();
  // callback to the failure checker when it is resumed, may reset some parameters
  virtual void OnResume();

private:
  // calls all the callbacks
  void ExecuteCallbacks();
  // Gets called to perform a check on the activity of the neighbor
  void Checkup();

  static const Time s_defaultCheckInterval;

  Neighbor m_neighbor; // the neighbor the failure detector is detector for
  std::vector<FailureCallback> m_failureCallbacks; // all the callbacks to be called on failure
  Time m_checkInterval; // interval for checking time
  bool m_suspended;
};

template<typename FailureDetectorType, typename ...Args>
std::shared_ptr<NeighborFailureDetector> MakeFailureDetector(Args... args) {
  auto detector = std::make_shared<FailureDetectorType>(std::forward<Args>(args)...);
  detector->Start();
  return detector;
}

class SimpleFailureDetector : public NeighborFailureDetector {
public:
  SimpleFailureDetector(Neighbor neighbor);
  SimpleFailureDetector(Neighbor neighhor, uint8_t maxMissed, Time helloInterval);
  virtual ~SimpleFailureDetector() = default;

  virtual void HelloReceived(const HelloHeader& header) override;

  uint8_t MaxMissed();
  void MaxMissed(uint8_t missedCount);

  Time HelloInterval();
  void HelloInterval(Time interval);
protected:
  virtual bool HasFailed() override;
  virtual void OnStart();
private:

  // configuration
  static const uint8_t s_maxMissedDefault;
  static const Time s_defaultHelloInterval;
  // max amount of intervals missed
  uint8_t m_maxMissed;
  // the supposed interval between hello packets
  Time m_helloInterval;

  // operation
  Time m_latestHello;
};

} // namespace ant_routing
} // namespace ns3

#endif
