#ifndef INPUT_STREAM_H_
#define INPUT_STREAM_H_

#include "base/base.h"

namespace io {
class MemoryBlock;

class InputStream {
  public:
    class Source {
      public:
        virtual ~Source() {
        }

        const std::vector<iovec>& iov() const {
          return iov_;
        }

      protected:
        void attatchData(iovec rhs) {
          iov_.push_back(rhs);
        }
        void attatchData(const std::vector<iovec>& rhs) {
          for (auto it = rhs.begin(); it != rhs.end(); ++it) {
            iov_.push_back(*it);
          }
        }
        void attatchData(const char* data, uint32 len) {
          iovec io;
          io.iov_base = const_cast<char*>(data);
          io.iov_len = len;
          iov_.push_back(io);
        }

      private:
        std::vector<iovec> iov_;
    };

    explicit InputStream(Source* reader, bool auto_release = true)
        : iov_(reader->iov()), auto_release_(auto_release) {
      reader_ = reader;
      index_ = offset_ = total_ = 0;
    }
    virtual ~InputStream() {
      if (auto_release_) {
        delete reader_;
      }
    }

    // return false iif no data can be read.
    // buf and len must not be null.
    bool read(const char** buf, uint64* len) const;

    void skip(uint64 len) const;
    void backup(uint64 len) const;

    // return the total number of bytes read since this object was created.
    int byteCount() const {
      return total_;
    }

  private:
    const std::vector<iovec> iov_;

    bool auto_release_;
    Source* reader_;

    mutable uint32 index_;
    mutable uint64 offset_;
    mutable uint64 total_;

    DISALLOW_COPY_AND_ASSIGN(InputStream);
};

class BytesSource : public InputStream::Source {
  public:
    BytesSource(char* data, uint32 len, bool auto_release = true)
        : data_(data), auto_release_(auto_release) {
      if (data != nullptr && len != 0) {
        attatchData(data_, len);
      }
    }
    virtual ~BytesSource() {
      if (auto_release_) {
        ::free(const_cast<char*>(data_));
      }
    }

  private:
    const char* data_;
    bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(BytesSource);
};

class ChunkSource : public InputStream::Source {
  public:
    explicit ChunkSource(MemoryBlock* chunk, bool auto_release = true);
    virtual ~ChunkSource();

  private:
    MemoryBlock* chunk_;
    bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ChunkSource);
};

class ConcatenaterSource : public InputStream::Source {
  public:
    typedef InputStream::Source SelfType;

    explicit ConcatenaterSource(SelfType* src = NULL) {
      if (src != NULL) {
        push(src);
      }
    }
    virtual ~ConcatenaterSource() {
      STLClear(&src_vec_);
    }

    void push(SelfType* src) {
      DCHECK_NOTNULL(src);
      const std::vector<iovec>& iov = src->iov();
      for (auto it = iov.begin(); it != iov.end(); ++it) {
        attatchData(*it);
      }
    }

  private:
    std::vector<SelfType*> src_vec_;

    DISALLOW_COPY_AND_ASSIGN(ConcatenaterSource);
};
}
#endif /* INPUT_BUF_H_ */
