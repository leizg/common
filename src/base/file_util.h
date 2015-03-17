#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include "macro_def.h"
#include "data_types.h"

#include <cstdio>
#include <dirent.h>

namespace detail {

class FileAbstruct {
  public:
    virtual ~FileAbstruct() {
    }

    const std::string& fpath() const {
      return fpath_;
    }

    virtual bool Init() = 0;

  protected:
    explicit FileAbstruct(const std::string& fpath)
        : fpath_(fpath) {
      DCHECK(!fpath.empty());
    }

    const std::string fpath_;

  private:
    DISALLOW_COPY_AND_ASSIGN(FileAbstruct);
};
}  // end for namespace named detail

void RemoveFile(const std::string& path);

bool IsDir(const std::string& path);
bool IsRegular(const std::string& path);
bool FileExist(const std::string& path);

bool FileSize(int fd, uint64* size);
bool FileSize(const std::string& path, uint64* size);

bool FlushData(int fd);
bool FlushFile(int fd);

bool FileTruncate(int fd, uint64 size);
bool FileTruncate(const std::string& path, uint64 size);

class SequentialReadonlyFile : public detail::FileAbstruct {
  public:
    explicit SequentialReadonlyFile(const std::string& fpath)
        : FileAbstruct(fpath), stream_(NULL) {
    }
    ~SequentialReadonlyFile() {
      if (stream_ != NULL) {
        ::fclose(stream_);
        stream_ = NULL;
      }
    }

    virtual bool Init();

    int32 read(char* buf, uint32 len);

  private:
    FILE *stream_;

    DISALLOW_COPY_AND_ASSIGN(SequentialReadonlyFile);
};

class RandomAccessFile : public detail::FileAbstruct {
  public:
    explicit RandomAccessFile(const std::string& fpath)
        : FileAbstruct(fpath), fd_(INVALID_FD) {
    }
    virtual ~RandomAccessFile() {
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

class writeableFile : public detail::FileAbstruct {
  public:
    explicit writeableFile(const std::string& fpath)
        : detail::FileAbstruct(fpath) {
    }
    virtual ~writeableFile() {
    }

    virtual bool Init() = 0;

    virtual bool flush() = 0;
    virtual int32 write(const char* buf, uint32 len) = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(writeableFile);
};

class AppendonlyFile : public writeableFile {
  public:
    explicit AppendonlyFile(const std::string& fpath)
        : writeableFile(fpath), stream_(NULL) {
    }
    virtual ~AppendonlyFile() {
      if (stream_ != NULL) {
        ::fclose(stream_);
        stream_ = NULL;
      }
    }

    virtual bool Init();

    virtual bool flush();
    virtual int32 write(const char* buf, uint32 len);

  private:
    FILE* stream_;

    DISALLOW_COPY_AND_ASSIGN(AppendonlyFile);
};

class AppendonlyMmapedFile : public writeableFile {
  public:
    explicit AppendonlyMmapedFile(const std::string& fpath, uint32 mapped_size =
                                      0)
        : writeableFile(fpath), fd_(INVALID_FD) {
      mem_ = pos_ = end_ = NULL;
      mapped_size_ = mapped_size == 0 ? kDefaultMappedSize : mapped_size;
      flushed_size_ = 0;
      mapped_offset_ = 0;
    }
    virtual ~AppendonlyMmapedFile();

    virtual bool Init();

    virtual bool flush();
    virtual int32 write(const char* buf, uint32 len);

  private:
    int fd_;

    char* mem_;
    char* pos_;
    char* end_;

    uint32 mapped_size_;
    uint32 flushed_size_;
    uint64 mapped_offset_;

    bool doMap();
    void unMap();

    const static uint32 kDefaultMappedSize = 8192;

    DISALLOW_COPY_AND_ASSIGN(AppendonlyMmapedFile);
};

// not threadsafe.
class DirIterator : public detail::FileAbstruct {
  public:
    enum Type {
      REG_FILE = 0, DIR_TYPE, SYMBOLIC_LINK,
    };

    explicit DirIterator(const std::string& path)
        : detail::FileAbstruct(path), dir_(NULL) {
    }
    virtual ~DirIterator() {
      if (dir_ != NULL) {
        ::closedir(dir_);
        dir_ = NULL;
      }
    }

    virtual bool Init();

    const std::string* next();
    const std::string* next(Type type);

  private:
    DIR* dir_;

    struct dirent entry_;
    std::string name_cache_;

    uint8 typeConvert(Type type) const;

    DISALLOW_COPY_AND_ASSIGN(DirIterator);
};

#endif
