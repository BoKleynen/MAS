/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ANTPACKET_H
#define ANTPACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
namespace ant_routing {

enum AntType
{
  ReactiveForwardAnt = 0,
  ProactiveForwardAnt = 1,
  ProactiveBroadcastAnt = 2,
  BackwardAnt = 3,
};

class ReactiveForwardAnt : public Header
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  /**
   * \returns the expected size of the header.
   *
   * This method is used by Packet::AddHeader
   * to store a header into the byte buffer of a packet. This method
   * should return the number of bytes which are needed to store
   * the full header data by Serialize.
   */
  uint32_t GetSerializedSize () const;
  /**
   * \param start an iterator which points to where the header should
   *        be written.
   *
   * This method is used by Packet::AddHeader to
   * store a header into the byte buffer of a packet.
   * The data written
   * is expected to match bit-for-bit the representation of this
   * header in a real network.
   */
  void Serialize (Buffer::Iterator start) const;
  /**
   * \param start an iterator which points to where the header should
   *        read from.
   * \returns the number of bytes read.
   *
   * This method is used by Packet::RemoveHeader to
   * re-create a header from the byte buffer of a packet.
   * The data read is expected to
   * match bit-for-bit the representation of this header in real
   * networks.
   *
   * Note that data is not actually removed from the buffer to
   * which the iterator points.  Both Packet::RemoveHeader() and
   * Packet::PeekHeader() call Deserialize(), but only the RemoveHeader()
   * has additional statements to remove the header bytes from the
   * underlying buffer and associated metadata.
   */
  uint32_t Deserialize (Buffer::Iterator start);
  /**
   * \param os output stream
   * This method is used by Packet::Print to print the
   * content of a header as ascii data to a c++ output stream.
   * Although the header is free to format its output as it
   * wishes, it is recommended to follow a few rules to integrate
   * with the packet pretty printer: start with flags, small field
   * values located between a pair of parens. Values should be separated
   * by whitespace. Follow the parens with the important fields,
   * separated by whitespace.
   * i.e.: (field1 val1 field2 val2 field3 val3) field4 val4 field5 val5
   */
  void Print (std::ostream &os) const;
private:
  Ipv4Address    dst;            ///< Destination IP Address
  Ipv4Address    origin;         ///< Originator IP Address
} // class ReactiveForwardAnt

class ProactiveForwardAnt : public Header
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  /**
   * \returns the expected size of the header.
   *
   * This method is used by Packet::AddHeader
   * to store a header into the byte buffer of a packet. This method
   * should return the number of bytes which are needed to store
   * the full header data by Serialize.
   */
  uint32_t GetSerializedSize () const;
  /**
   * \param start an iterator which points to where the header should
   *        be written.
   *
   * This method is used by Packet::AddHeader to
   * store a header into the byte buffer of a packet.
   * The data written
   * is expected to match bit-for-bit the representation of this
   * header in a real network.
   */
  void Serialize (Buffer::Iterator start) const;
  /**
   * \param start an iterator which points to where the header should
   *        read from.
   * \returns the number of bytes read.
   *
   * This method is used by Packet::RemoveHeader to
   * re-create a header from the byte buffer of a packet.
   * The data read is expected to
   * match bit-for-bit the representation of this header in real
   * networks.
   *
   * Note that data is not actually removed from the buffer to
   * which the iterator points.  Both Packet::RemoveHeader() and
   * Packet::PeekHeader() call Deserialize(), but only the RemoveHeader()
   * has additional statements to remove the header bytes from the
   * underlying buffer and associated metadata.
   */
  uint32_t Deserialize (Buffer::Iterator start);
  /**
   * \param os output stream
   * This method is used by Packet::Print to print the
   * content of a header as ascii data to a c++ output stream.
   * Although the header is free to format its output as it
   * wishes, it is recommended to follow a few rules to integrate
   * with the packet pretty printer: start with flags, small field
   * values located between a pair of parens. Values should be separated
   * by whitespace. Follow the parens with the important fields,
   * separated by whitespace.
   * i.e.: (field1 val1 field2 val2 field3 val3) field4 val4 field5 val5
   */
  void Print (std::ostream &os) const;
private:
  Ipv4Address    dst;            ///< Destination IP Address
  Ipv4Address    origin;         ///< Originator IP Address
} // class ProactiveForwardAnt

} // namespace ant_routing
} // namespace
#endif /* ANTPACKET_H */
