#ifndef _GVM_DISK_H_
#define _GVM_DISK_H_

#include <cstdint>
#include <string>

namespace gvm {
class Disk {
 public:
  constexpr static uint32_t kSectorSize = 512 /* bytes */;

  Disk() {}
  virtual ~Disk() {}

  // If init is successful, it should not fail if called multiple times.
  virtual bool Init() = 0;

  virtual int32_t Read(
      uint32_t start_sector, uint32_t sector_count, uint32_t* mem) const = 0;
  virtual int32_t Write(
      uint32_t start_sector, uint32_t sector_count, const uint32_t* mem) = 0;
};


class FileBackedDisk : public Disk {
 public:
  FileBackedDisk(const std::string& file_name, uint32_t sector_count)
    : file_name_(file_name), sector_count_(sector_count), map_(nullptr) {}
  ~FileBackedDisk() override;

  bool Init() override;

  int32_t Read(
      uint32_t start_sector, uint32_t sector_count, uint32_t* mem) const override;
  int32_t Write(
      uint32_t start_sector, uint32_t sector_count, const uint32_t* mem) override;

 private:
  std::string file_name_;
  const uint32_t sector_count_;
  uint32_t* map_;
  int file_descriptor_;
};

}  // namespace gvm

#endif  // _GVM_DISK_H_
