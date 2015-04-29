#pragma once

#include "base/base.h"

namespace async {

DEFINE_uint64(max_open_files, 10240, "max number of files can be opened");
DEFINE_bool(enable_oom, false, "disable oom");
DEFINE_int32(oom, -17, "oom value");

class Env {
  public:
    Env() {
      // maybe false, iif not root.
      setOom();
      setMaxOpenFiles();
    }

  private:
    bool setMaxOpenFiles() const {
      struct rlimit l;
      l.rlim_cur = FLAGS_max_open_files;
      l.rlim_max = FLAGS_max_open_files;
      int ret = ::setrlimit(RLIMIT_NOFILE, &l);
      if (ret != 0) {
        PLOG(WARNING)<< "setrlimit error";
        return false;
      }
      DLOG(INFO)<<"set max_open_files: " << FLAGS_max_open_files;
      return true;
    }

    const std::string OomPath() const {
      char buf[4096];
      ::memset(buf, 0x00, sizeof(buf));
      int pos = snprintf(buf, sizeof("/proc/"), "%s", "/proc/");
      pos += snprintf(buf + pos, sizeof(buf) - pos, "%i", ::getpid());
      pos += snprintf(buf + pos, sizeof(buf) - pos, "%s", "/oom_adj");
      return std::string(buf, pos);
    }

    bool setOom() const {
      if (FLAGS_enable_oom) {
        char buf[1024];
        ::memset(buf, 0x00, sizeof(buf));
        int pos = 0;
        if (FLAGS_oom < 0) {
          pos += ::snprintf(buf, sizeof(buf), "%s", "-");
        }
        pos += ::snprintf(buf + pos, sizeof(buf), "%u", ::abs(FLAGS_oom));
        pos += ::snprintf(buf + pos, sizeof(buf), "%s", "\n");
        if (!writeFile(OomPath(), std::string(buf, pos))) {
          PLOG(WARNING)<<"update oom failed...";
          return false;
        }
      }
      return true;
    }
  };
}
