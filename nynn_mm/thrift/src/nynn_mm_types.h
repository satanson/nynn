/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef nynn_mm_TYPES_H
#define nynn_mm_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>



namespace nynn { namespace mm {


class xchunk_entry_t {
 public:

  static const char* ascii_fingerprint; // = "3F1101A329696BD4B88B9212A3D74DCD";
  static const uint8_t binary_fingerprint[16]; // = {0x3F,0x11,0x01,0xA3,0x29,0x69,0x6B,0xD4,0xB8,0x8B,0x92,0x12,0xA3,0xD7,0x4D,0xCD};

  xchunk_entry_t() : _path(), _flag(0), _minvtx(0), _maxvtx(0), _where(0) {
  }

  virtual ~xchunk_entry_t() throw() {}

  std::string _path;
  int8_t _flag;
  int64_t _minvtx;
  int64_t _maxvtx;
  int32_t _where;

  void __set__path(const std::string& val) {
    _path = val;
  }

  void __set__flag(const int8_t val) {
    _flag = val;
  }

  void __set__minvtx(const int64_t val) {
    _minvtx = val;
  }

  void __set__maxvtx(const int64_t val) {
    _maxvtx = val;
  }

  void __set__where(const int32_t val) {
    _where = val;
  }

  bool operator == (const xchunk_entry_t & rhs) const
  {
    if (!(_path == rhs._path))
      return false;
    if (!(_flag == rhs._flag))
      return false;
    if (!(_minvtx == rhs._minvtx))
      return false;
    if (!(_maxvtx == rhs._maxvtx))
      return false;
    if (!(_where == rhs._where))
      return false;
    return true;
  }
  bool operator != (const xchunk_entry_t &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const xchunk_entry_t & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(xchunk_entry_t &a, xchunk_entry_t &b);


class xinetaddr_t {
 public:

  static const char* ascii_fingerprint; // = "F3540C99C9016F618854ABDC57D34F96";
  static const uint8_t binary_fingerprint[16]; // = {0xF3,0x54,0x0C,0x99,0xC9,0x01,0x6F,0x61,0x88,0x54,0xAB,0xDC,0x57,0xD3,0x4F,0x96};

  xinetaddr_t() : _hostname(), _hostaddr(0), _port(0) {
  }

  virtual ~xinetaddr_t() throw() {}

  std::string _hostname;
  int32_t _hostaddr;
  int16_t _port;

  void __set__hostname(const std::string& val) {
    _hostname = val;
  }

  void __set__hostaddr(const int32_t val) {
    _hostaddr = val;
  }

  void __set__port(const int16_t val) {
    _port = val;
  }

  bool operator == (const xinetaddr_t & rhs) const
  {
    if (!(_hostname == rhs._hostname))
      return false;
    if (!(_hostaddr == rhs._hostaddr))
      return false;
    if (!(_port == rhs._port))
      return false;
    return true;
  }
  bool operator != (const xinetaddr_t &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const xinetaddr_t & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(xinetaddr_t &a, xinetaddr_t &b);


class xletter_t {
 public:

  static const char* ascii_fingerprint; // = "66300C0BD0E5C868AF1BEF652F664894";
  static const uint8_t binary_fingerprint[16]; // = {0x66,0x30,0x0C,0x0B,0xD0,0xE5,0xC8,0x68,0xAF,0x1B,0xEF,0x65,0x2F,0x66,0x48,0x94};

  xletter_t() {
  }

  virtual ~xletter_t() throw() {}

  std::vector<xinetaddr_t>  _iaddr_table;
  std::vector<xchunk_entry_t>  _chk_table;

  void __set__iaddr_table(const std::vector<xinetaddr_t> & val) {
    _iaddr_table = val;
  }

  void __set__chk_table(const std::vector<xchunk_entry_t> & val) {
    _chk_table = val;
  }

  bool operator == (const xletter_t & rhs) const
  {
    if (!(_iaddr_table == rhs._iaddr_table))
      return false;
    if (!(_chk_table == rhs._chk_table))
      return false;
    return true;
  }
  bool operator != (const xletter_t &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const xletter_t & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(xletter_t &a, xletter_t &b);

}} // namespace

#endif