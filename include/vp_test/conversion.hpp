#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

namespace testing::conversion {

void    int32_to_bytes(uint32_t value, uint8_t *buffer, std::size_t start);
uint32_t bytes_to_int32(const uint8_t *buffer, std::size_t start);
void    int64_to_bytes(int64_t value, uint8_t *buffer, std::size_t start);
int64_t bytes_to_int64(const uint8_t *buffer, std::size_t start);
bool    check_cast_to_uint32(uint64_t value);


class BufferWriter{
  public:
  BufferWriter(std::vector<uint8_t>& data)
  : data_(data),
    pos_(0){}

  bool write_int32(uint32_t in){
    if(pos_ + sizeof(uint32_t) > data_.size())
      data_.resize(pos_ + sizeof(uint32_t));

    int32_to_bytes(in, data_.data(), pos_);
    pos_ += sizeof(uint32_t);

    return true;
  }

  bool write_uint8(uint8_t in){
    if(pos_ + 1 > data_.size())
      data_.resize(pos_ + 1);

    data_[pos_++] = in;
    return true;
  }

  // This works both with strings and string_view
  bool write_string(std::string_view str){
    if (pos_ + str.size() > data_.size())
        data_.resize(pos_ + str.size());

    memcpy(data_.data() + pos_, str.data(), str.size());
    pos_ += str.size();
    return true;
  }


  private:
  std::vector<uint8_t>& data_;
  size_t pos_;

};


class BufferReader{
  public:
  BufferReader(std::vector<uint8_t>& data)
  : data_(data),
    pos_(0){}

  // This fills the out (int) field with 4 bytes of the data vector
  // Which bytes? 0-3 for the 1st call, 4-7 for the 2nd call etc
  bool read_int32(uint32_t& out){
    if(pos_ + sizeof(uint32_t) > data_.size())
      return false;
    
    out = bytes_to_int32(data_.data(), pos_);
    pos_ += sizeof(uint32_t);
    return true;
  }

  // This fills the out (uint8_t) field with 1 byte of the data vector
  // Which bytes? 0 for the 1st call, 1 for the 2nd call etc
  bool read_uint8(uint8_t& out){
    if(pos_ + 1 > data_.size())
      return false;
    
    out = data_[pos_++];
    return true;
  }

  // This fills the out (string) field with len bytes of the data vector
  // Which bytes? 0 - len for the 1st call
  bool read_string(std::string& out, uint8_t len){
    if(pos_ + len > data_.size())
        return false;

    if(len == 0)
      return true; // Nothing to do :)

    out = std::string(reinterpret_cast<const char *>(data_.data()) + pos_, len);
    pos_ += len;
    return true;
  }

  size_t remaining() const {
    return data_.size() - pos_;
  }

  private:
  std::vector<uint8_t>& data_;
  size_t pos_;
};

}  // namespace testing::conversion

#endif  // !CONVERSION_HPP
