#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "base/base.h"

namespace util {

class Compression {
  public:
    virtual ~Compression() {
    }

    // it is here for zero copy
    class InBuf {
      public:
        virtual ~InBuf() {
        }

        virtual uint64 size() = 0;

        virtual bool read(char** buf, size_t* size) = 0;
    };

    class OutBuf {
      public:
        virtual ~OutBuf() {
        }

        virtual void ensureLeft(uint32 space) = 0;

        virtual void write(char** buf, size_t size) = 0;
        virtual void backUp(size_t size) = 0;
    };

    virtual bool compress(InBuf* in_buf, OutBuf* out_buf) = 0;
    virtual bool decompress(InBuf* in_buf, OutBuf* out_buf) = 0;

  protected:
    Compression() {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Compression);
};

class StringInBuf : public Compression::InBuf {
  public:
    explicit StringInBuf(const std::string* str)
        : index_(0) {
      data_.push_back(str);
    }
    explicit StringInBuf(const std::vector<const std::string*>& data)
        : index_(0), data_(data) {
    }
    virtual ~StringInBuf() {
    }

    virtual uint64 size() {
      uint64 size = 0;
      for (uint32 i = 0; i < data_.size(); ++i) {
        const std::string* s = data_[i];
        size += s->size();
      }
      return size;
    }

    virtual bool read(char** buf, size_t* size) {
      if (index_ != data_.size()) {
        const std::string* s = data_[index_++];
        *buf = const_cast<char*>(s->data());
        *size = s->size();
        return true;
      }

      return false;
    }

  private:
    int index_;
    std::vector<const std::string*> data_;

    DISALLOW_COPY_AND_ASSIGN(StringInBuf);
};

class StringOutBuf : public Compression::OutBuf {
  public:
    StringOutBuf() {
    }
    virtual ~StringOutBuf() {
    }

    virtual void ensureLeft(uint32 space) {
//      data_.resize(data_.size() + space);
    }

    virtual void write(char** buf, size_t size) {
      uint32 old_size = data_.size();
      data_.resize(old_size + size);

      *buf = const_cast<char*>(data_.c_str() + old_size);
    }
    virtual void backUp(size_t size) {
      if (data_.size() > size) {
        data_.resize(data_.size() - size);
      }
    }

    const std::string& data() const {
      return data_;
    }

  private:
    std::string data_;

    DISALLOW_COPY_AND_ASSIGN(StringOutBuf);
};
}
#endif /* COMPRESSION_H_ */
