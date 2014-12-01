#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include "macro_def.h"
#include "data_types.h"

#include <cstdio>

namespace detail {

class DiskFile {
  public:
    virtual ~DiskFile() {
    }

    const std::string& fpath() const {
      return fpath_;
    }

    virtual bool Init() = 0;

  protected:
    explicit DiskFile(const std::string& fpath)
        : fpath_(fpath) {
      DCHECK(!fpath.empty());
    }

    const std::string fpath_;

  private:
    DISALLOW_COPY_AND_ASSIGN(DiskFile);
};
}  // end for namespace named detail

bool IsDir(const std::string& path);
bool IsRegular(const std::string& path);
bool FileExist(const std::string& path);

bool FileSize(int fd, uint64* size);
bool FileSize(const std::string& path, uint64* size);

bool FlushData(int fd);
bool FlushFile(int fd);

bool FileTruncate(int fd, uint64 size);
bool FileTruncate(const std::string& path, uint64 size);

class SequentialAccessFile : public detail::DiskFile {
  public:
    explicit SequentialAccessFile(const std::string& fpath)
        : DiskFile(fpath), stream_(NULL) {
    }
    ~SequentialAccessFile() {
      if (stream_ != NULL) {
        ::fclose(stream_);
        stream_ = NULL;
      }
    }

    virtual bool Init();

    int32 read(char* buf, uint32 len);

  private:
    FILE *stream_;

    DISALLOW_COPY_AND_ASSIGN(SequentialAccessFile);
};

class RandomAccessFile : public detail::DiskFile {
  public:
    RandomAccessFile(const std::string& fpath)
        : DiskFile(fpath), fd_(INVALID_FD) {
    }
    ~RandomAccessFile() {
      closeWrapper(fd_);
    }

    virtual bool Init();

    int32 read(char* buf, uint32 len, off_t offset);

    bool flush(bool only_flush_data = true);
    int32 write(const char* buf, uint32 len, off_t offset);

  private:
    int fd_;

    DISALLOW_COPY_AND_ASSIGN(RandomAccessFile);
};

class AppendonlyMmapedFile : public detail::DiskFile {
  public:
    explicit AppendonlyMmapedFile(const std::string& fpath)
        : DiskFile(fpath), fd_(INVALID_FD) {
      mem_ = pos_ = end_ = NULL;

      flushed_size_ = 0;
      mapped_offset_ = 0;
    }
    virtual ~AppendonlyMmapedFile();

    bool Init();

    void flush();
    int32 write(const char* buf, uint32 len);

  private:
    int fd_;

    char* mem_;
    char* pos_;
    char* end_;

    uint32 flushed_size_;
    uint64 mapped_offset_;

    bool doMap();
    void unMap();

    const static uint32 kMappedSize = 8192;

    DISALLOW_COPY_AND_ASSIGN(AppendonlyMmapedFile);
};

#endif
