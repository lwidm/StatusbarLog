// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 Lukas Widmer
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// -- statusbarlog/src/sink.cc

// clang-format off

#include "statusbarlog/sink.h"
#include <cstdio>
#include <cstring>
#include <ios>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>

// clang-format on

namespace statusbar_log {
namespace sink {

int SinkInitStdout(Sink& sink) {
  sink.type = kSinkStdout;
  sink.out = &std::cout;
  sink.owned_file.reset();
  sink.fd = fileno(stdout);
  return 0;
}

int SinkInitFile(Sink& sink, const std::string& path) {
  try {
    std::unique_ptr<std::ofstream> f =
        std::make_unique<std::ofstream>(path, std::ios::app);
    if (!f->is_open() || !f->good()) {
      return -1;
    }
    sink.type = kSinkFileOwned;
    sink.out = f.get();
    sink.owned_file = std::move(f);
    sink.fd = -1;
  } catch (...) {
    return -2;
  }
  return 0;
}

int SinkInitOstream(Sink& sink, std::ostream& os) {
  sink.type = kSinkOstreamWrapped;
  sink.out = &os;
  sink.owned_file.reset();
  if (&os == &std::cout) {
    sink.fd = fileno(stdout);
  } else if (&os == &std::cerr) {
    sink.fd = fileno(stderr);
  } else {
    sink.fd = -1;
  }
  return 0;
}

ssize_t SinkWrite(Sink& sink, const char* buf, std::size_t len) {
  if (!buf) return -1;

  std::lock_guard<std::mutex> lock(sink.mutex);

  if (len == 0) return 0;
  if (!sink.out->good() && sink.fd < 0) return -1;

  if (sink.fd >= 0) {
#if defined(SSIZE_MAX)
    if (len > static_cast<std::size_t>(SSIZE_MAX)) return -2;
#endif
    ssize_t rc = ::write(sink.fd, buf, static_cast<size_t>(len));
    return rc;
  }

  if (!sink.out->good()) return -3;

  if (len >
      static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
    return -4;
  }

  std::streambuf* sb = sink.out->rdbuf();
  if (!sb) return -5;

  std::streamsize want = static_cast<std::streamsize>(len);
  std::streamsize written = sb->sputn(buf, want);

  if (written != want) {
    sink.out->setstate(std::ios::failbit);
    return -6;
  }
  return static_cast<ssize_t>(written);
}

ssize_t SinkWriteStr(Sink& sink, const std::string& str) {
  return SinkWrite(sink, str.c_str(), str.size());
}

int SinkFlush(Sink& s) {
  std::lock_guard<std::mutex> lk(s.mutex);
  if (s.fd >= 0) {
    return 0;
  }
  if (!s.out->good()) return -1;
  s.out->flush();
  return s.out->good() ? 0 : -2;
}

void SinkClose(Sink& sink) {
  std::lock_guard<std::mutex> lk(sink.mutex);
  if (sink.owned_file) {
    sink.owned_file->flush();
    sink.owned_file.reset();  // closes file
  }
  sink.out = nullptr;
  sink.fd = -1;
  sink.type = kSinkInvalid;
}

bool SinkIsTty(Sink& sink) {
  if (sink.fd >= 0) {
    return ::isatty(sink.fd) != 0;
  }
  if (sink.out == &std::cout) return ::isatty(fileno(stdout));
  if (sink.out == &std::cerr) return ::isatty(fileno(stderr));
  return false;
}

}  // namespace sink
}  // namespace statusbar_log
