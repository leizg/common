#include "file_util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace {
bool openFile(const std::string& fpath, int* fd, int flag) {
  int ret = ::open(fpath.c_str(), flag);
  if (ret == INVALID_FD) {
    PLOG(WARNING)<< "open error, path: " << fpath;
    return false;
  }

  *fd = ret;
  return true;
}

bool Stat(const std::string& path, struct stat* st) {
  int fd;
  if (!openFile(path, &fd, O_RDONLY)) return false;

  int ret = ::fstat(fd, st);
  closeWrapper(fd);
  if (ret != -1) {
    PLOG(WARNING)<< "fstat error, path: " << path;
    return true;
  }

  return false;
}
}

bool FileExist(const std::string& path) {
  return ::access(path.c_str(), F_OK) == 0;
}

bool IsDir(const std::string& path) {
  struct stat st;
  if (!Stat(path, &st)) return false;
  return S_ISDIR(st.st_mode);
}

bool IsRegular(const std::string& path) {
  struct stat st;
  if (!Stat(path, &st)) return false;
  return S_ISREG(st.st_mode);
}

bool FileSize(int fd, uint64* size) {
  struct stat st;
  int ret = ::fstat(fd, &st);
  if (ret != -1) {
    *size = st.st_size;
    return true;
  }

  return false;
}

bool FileSize(const std::string& path, uint64* size) {
  int fd = INVALID_FD;
  if (!openFile(path, &fd, O_RDONLY)) return false;

  bool ret = FileSize(fd, size);
  closeWrapper(fd);
  return ret;
}

bool FlushData(int fd) {
  int ret = ::fdatasync(fd);
  if (ret == -1) {
    PLOG(WARNING)<< "fdatasync error, fd: " << fd;
    return false;
  }
  return true;
}

bool FlushFile(int fd) {
  int ret = ::fsync(fd);
  if (ret == -1) {
    PLOG(WARNING)<< "fsync error, fd: " << fd;
    return false;
  }
  return true;
}

bool FileTruncate(int fd, uint64 size) {
  int ret = ::ftruncate(fd, size);
  if (ret != 0) {
    LOG(WARNING)<< "ftruncate error, fd: " << fd;
    return false;
  }
  return true;
}

bool FileTruncate(const std::string& path, uint64 size) {
  int ret = ::truncate(path.c_str(), size);
  if (ret != 0) {
    LOG(WARNING)<< "ftruncate error, path: " << path;
    return false;
  }
  return true;
}

bool SequentialAccessFile::Init() {
  if (stream_ != NULL) return false;

  stream_ = ::fopen(fpath_.c_str(), "r");
  if (stream_ == NULL) {
    PLOG(WARNING)<< "fopen error, path: " << fpath_;
    return false;
  }
  return true;
}

int32 SequentialAccessFile::read(char* buf, uint32 len) {
  DCHECK_GE(len, 0);
  int32 readn = ::fread(buf, len, 1, stream_);
  if (readn == -1) {
    PLOG(WARNING)<< "fread error, path: " << fpath_;
    return -1;
  }

  return readn;
}

bool RandomAccessFile::Init() {
  return openFile(fpath_, &fd_, O_RDWR);
}

int32 RandomAccessFile::read(char* buf, uint32 len, off_t offset) {
  uint32 left = len;
  while (left > 0) {
    int32 readn = ::pread(fd_, buf, left, offset);
    if (readn == 0) return 0;
    else if (readn == -1) {
      if (errno == EINTR) continue;
      PLOG(WARNING)<< "pread error, path: " << fpath_;
      return -1;
    }

    buf += readn;
    left -= readn;
    offset += readn;
  }
  return len - left;
}

bool RandomAccessFile::flush(bool only_flush_data) {
  if (fd_ != INVALID_FD) {
    if (only_flush_data) return FlushData(fd_);
    return FlushFile(fd_);
  }
  return false;
}

int32 RandomAccessFile::write(const char* buf, uint32 len, off_t offset) {
  uint32 left = len;
  while (left > 0) {
    int32 writen = ::pwrite(fd_, buf, left, offset);
    if (writen == -1) {
      if (errno == EINTR) continue;
      PLOG(WARNING)<< "pread error, path: " << fpath_;
      return -1;
    }

    buf += writen;
    left -= writen;
    offset += writen;
  }
  return len - left;
}

AppendonlyMmapedFile::~AppendonlyMmapedFile() {
  if (fd_ != INVALID_FD) {
    if (pos_ != end_) {
      FileTruncate(fd_, mapped_offset_ - (end_ - pos_));
    }

    closeWrapper(fd_);
  }
}

bool AppendonlyMmapedFile::Init() {
  if (!openFile(fpath_, &fd_, O_APPEND)) return false;

  if (!FileSize(fd_, &mapped_offset_)) {
    closeWrapper(fd_);
    return false;
  }
  return true;
}

void AppendonlyMmapedFile::flush() {
  DCHECK_GE(end_, mem_);
  if (fd_ != INVALID_FD && end_ != mem_) {
    int32 size = end_ - mem_ - flushed_size_;
    if (size > 0) {
      int ret = ::msync(mem_ + flushed_size_, size, MS_SYNC);
      if (ret != 0) {
        PLOG(WARNING)<< "msync error";
        return;
      }
      flushed_size_ += size;
    }
  }
}

int32 AppendonlyMmapedFile::write(const char* buf, uint32 len) {
  if (fd_ == INVALID_FD) return -1;

  uint32 left = len;
  while (left > 0) {
    uint32 avail_size = end_ - pos_;
    if (avail_size == 0) {
      doMap();
      continue;
    }

    uint32 writen = std::min(avail_size, left);
    if (writen > 0) {
      ::memcpy(pos_, buf, writen);
      pos_ += writen;
      buf += writen;
      left -= writen;
    }
  }

  return len - left;
}

bool AppendonlyMmapedFile::doMap() {
  if (mem_ != NULL) unMap();
  if (!FileTruncate(fd_, mapped_offset_ + kMappedSize)) return false;

  mem_ = (char*) ::mmap64(NULL, kMappedSize, PROT_WRITE, MAP_SHARED, fd_,
                          mapped_offset_);
  if (mem_ == MAP_FAILED) {
    PLOG(WARNING)<< "mmap64 error, fd: " << fd_;
    return false;
  }

  pos_ = mem_;
  end_ = mem_ + kMappedSize;

  flushed_size_ = 0;
  mapped_offset_ += kMappedSize;

  return true;
}

void AppendonlyMmapedFile::unMap() {
  if (mem_ != NULL) {
    ::munmap(mem_, kMappedSize);
    mem_ = pos_ = end_ = NULL;
    flushed_size_ = 0;
  }
}

bool DirIterator::Init() {
  if (dir_ != NULL) return false;

  dir_ = ::opendir(fpath_.c_str());
  if (dir_ == NULL) {
    PLOG(WARNING)<< "opendir error, path: " << fpath_;
    return false;
  }
  return true;
}

uint8 DirIterator::typeConvert(Type type) const {
  switch (type) {
    case REG_FILE:
      return DT_REG;
    case DIR_TYPE:
      return DT_DIR;
    case SYMBOLIC_LINK:
      return DT_LNK;
  }
}

const std::string* DirIterator::next(Type type) {
  uint8 entry_type = typeConvert(type);
  while (next() != NULL) {
    if (entry_.d_type & entry_type) {
      return &name_cache_;
    }
  }
  return NULL;
}

const std::string* DirIterator::next() {
  if (dir_ != NULL) {
    struct dirent* dummy;
    int ret = ::readdir_r(dir_, &entry_, &dummy);
    if (ret != 0) {
      PLOG(WARNING)<< "readdir_r error, path: " << fpath_;
      return NULL;
    }

    if (dummy != NULL) {
      name_cache_ = entry_.d_name;
      return &name_cache_;
    }
  }

  return NULL;
}
