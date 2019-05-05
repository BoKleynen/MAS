/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANT_ROUTING_TABLE_H
#define ANT_ROUTING_TABLE_H

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
    const T& getValue(){
      return ref;
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
  /**
   * insert a new element in the routing table, in case
   * the entry was already present, the old one is replaced
   */
  void Insert(Ipv4Address addr, RoutingTableEntry&& entry) {
    m_table[addr] = std::move(entry);
  }

  void Insert(Ipv4Address addr, const RoutingTableEntry& entry) {
    m_table[addr] = entry;
  }

  OptEntry Lookup(Ipv4Address addr) {
    if(!HasEntry(addr)) {
      return OptEntry();
    }

    return OptEntry(m_table[addr]);
  }

  bool HasEntry(Ipv4Address addr) {
    return m_table.find(addr) != m_table.end();
  }

  /**
   * Updates an entry in the routing table using the
   * Update(Args...) function defined by RoutingTableEntry,
   */
  template<typename... Args>
  bool Update(Ipv4Address addr, Args&& ...args) {

    auto entry = Lookup(addr);

    return entry.doOptional(&RoutingTableEntry::Update, std::forward<Args>(args)...);
  }

  void Delete(Ipv4Address addr) {
    m_table.erase(addr);
  }
  // default value is empty
  static RoutingTable defaultVal = RoutingTable();

  decltype(auto) begin() {
    return m_table.begin();
  }

  decltype(auto) end() {
    return m_table.end();
  }

  decltype(auto) cbegin() const {
    return m_table.cbegin();
  }

  decltype(auto) cend() const {
    return m_table.cend();
  }
private:
  std::map<Ipv4Address, RoutingTableEntry> m_table;
};

} // namespace ant_routing
} // namespace ns3

#endif
