/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_ROUTING_TABLE_OLD_H
#define ANT_ROUTING_TABLE_OLD_H

#include <map>
#include <utility>
#include <functional>
#include "ns3/ipv4-address.h"
#include "ns3/core-module.h"

namespace ns3 {

  /**
   * Named requirements for optionals
   * static member: defaultVal
   */
  template<typename T>
  class Optional {
  public:
    Optional() : m_ref(T::defaultVal), m_ok(false) {}
    Optional(T& ref) : m_ref(ref), m_ok(true) {}

    // copy operations
    Optional(Optional& opt) : m_ref(opt.ref()), m_ok(opt.ok()){}
    Optional& operator=(Optional& opt) {
      m_ref = opt.ref();
      m_ok  = opt.ok();
      return *this;
    }

    // move operations
    Optional(Optional&& opt) : m_ref(opt.ref()), m_ok(opt.ok()) {}
    Optional& operator=(Optional&& opt) {
      m_ref = opt.ref();
      m_ok = opt.ok();
      return *this;
    }

    T& ref() {
      return m_ref;
    }

    bool ok() {
      return m_ok;
    }

    /**
     * returns the value inside the reference, in case the reference
     * is invalid, returns the default value.
     */
    const T& Value() {
      return m_ref;
    }

    /**
     * performs the operation provided on the enclosed reference
     * in case the reference is valid.
     */
    template<typename Method, typename ...Args>
    bool doOptional(Method method, Args&& ...args) {
      if (ok()) {
        (ref().*method)(std::forward<Args>(args)...);
      }
      return ok();
    }


  private:
    T& m_ref;
    bool m_ok;
  };


namespace ant_routing {


/**
 * Template class to create a routing table
 * Named requirements for a RoutingTableEntry
 * contains constructors:
 *  - default constructor;
 *
 * contains methods:
 *  - void Update(Args...); which is used to update the entry
 *  - operator<<(ostream& os, RoutingTableEntry re); for printing the routing table
 * note: declare the move constructor noexcept for faster copy
 *       semantics
 */
template<typename RoutingTableEntry>
class RoutingTable {
public:
  using OptEntry = Optional<RoutingTableEntry>;

  RoutingTable() : m_table(std::map<Ipv4Address, RoutingTableEntry>()) {}

  /**
   * insert a new element in the routing table, in case
   * the entry was already present, the old one is replaced
   */
  RoutingTableEntry& Insert(Ipv4Address addr, RoutingTableEntry&& entry) {
    m_table[addr] = std::move(entry);
    return m_table[addr];
  }

  RoutingTableEntry& Insert(Ipv4Address addr, const RoutingTableEntry& entry) {
    m_table[addr] = entry;
    return m_table[addr];
  }

  OptEntry Lookup(Ipv4Address addr) {
    if(!HasEntry(addr)) {
      NS_LOG_UNCOND("Invalid lookup done on: " << addr);
      return OptEntry();
    }

    NS_LOG_UNCOND("Valid lookup done on: " << addr);

    return OptEntry(m_table[addr]);
  }

  bool HasEntry(Ipv4Address addr) {
    NS_LOG_UNCOND("Size of m_table" << m_table.size());
    return m_table.find(addr) != m_table.end();
  }

  /**
   * Updates an entry in the routing table using the
   * Update(Args...) function defined by RoutingTableEntry,
   */
  template<typename... Args>
  void Update(Ipv4Address addr, Args&& ...args) {
    OptEntry entry = Lookup(addr);
    entry.doOptional(&RoutingTableEntry::Update, std::forward<Args>(args)...);
  }

  void Delete(Ipv4Address addr) {
    m_table.erase(addr);
  }

  std::vector<Ipv4Address> GetKeys() {
    std::vector<Ipv4Address> keys;
    for(auto iter = m_table.begin(); iter != m_table.end(); iter++) {
      keys.push_back(iter->first);
    }

    return keys;
  }

  // default value is empty
  static RoutingTable defaultVal;// = RoutingTable<RoutingTableEntry>();


private:
  std::map<Ipv4Address, RoutingTableEntry> m_table;
  // m_table must be declared before we can use it in decltype
public:
  auto begin() -> decltype(m_table.begin()){
    return m_table.begin();
  }

  auto end() -> decltype(m_table.end()) {
    return m_table.end();
  }

  auto cbegin() -> decltype(m_table.cbegin()) const {
    return m_table.cbegin();
  }

  auto cend() -> decltype(m_table.cend()) const {
    return m_table.cend();
  }
};

template<typename RoutingTableEntry>
RoutingTable<RoutingTableEntry> RoutingTable<RoutingTableEntry>::defaultVal = RoutingTable<RoutingTableEntry>();

} // namespace ant_routing
} // namespace ns3

#endif
