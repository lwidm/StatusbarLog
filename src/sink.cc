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
#include <vector>

#include "statusbarlog/statusbarlog.h"

// clang-format on

const std::string kFilename = "statusbarlog.cpp";

namespace statusbar_log {
namespace sink {

namespace {

/**
 *
 *
 * TODO : write this
 */
typedef struct {
  std::mutex mutex;
  std::ostream* out;
  std::unique_ptr<std::ofstream> owned_file;
  SinkType type;
  int fd;
  unsigned int id;
} Sink;

std::vector<std::unique_ptr<Sink>> _sink_registry = {};
std::vector<SinkHandle> _sink_free_handles = {};
unsigned int _sink_handle_id_count = 0;

static std::mutex _sink_registry_mutex;
static std::mutex _sink_id_count_mutex;

/**
 * \brief Check if the argument is a valid sink handle
 *
 * This functions performs a test on a SinkHandle and returns
 * statusbar_log::kStatusbarLogSuccess (i.e. 0) if it is a valid handle and a
 * negative number otherwise
 *
 * \param[in] SinkHandle struct to be checked for validity
 *
 * \return Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) if the handle is
 * valid, or one of these status codes:
 *         -  statusbar_log::kStatusbarLogSuccess (i.e. 0): Valid handle
 *         - -1: Invalid handle: Valid flag of handle set to false
 *         - -2: Invalid handle: Handle index out of bounds in
 * `statusbar_registry`
 *         - -3: Invalid handle: Handle IDs don't match between handle struct
 *         - -4: Invalid handle: Handle ID is 0 (i.e. invalid)
 * and registry
 *
 * \see _IsValidHandleVerbose: Verbose version of this function
 */
int _IsValidHandle(const SinkHandle& sink_handle) {
  const std::size_t idx = sink_handle.idx;

  if (!sink_handle.valid) {
    return -1;
  }

  if ((sink_handle.idx >= _sink_registry.size()) ||
      (sink_handle.idx == SIZE_MAX)) {
    return -2;
  }
  if (sink_handle.id != _sink_registry[idx]->id) {
    return -3;
  }
  if (sink_handle.id == 0) {
    return -4;
  }
  return kStatusbarLogSuccess;
}

/**
 * \brief Check if the argument is a valid sink handle and prints an error
 * message.
 *
 * This functions performs a test on a SinkHandle and returns
 * statusbar_log::kStatusbarLogSuccess (i.e. 0) if it is a valid handle and a
 * negative number otherwise. Same as _IsValidHandle but also prints an error
 * message.
 *
 * \param[in] SinkHandle struct to be checked for validity
 *
 * \return Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) if the handle is
 * valid, or one of these status codes:
 *         -  statusbar_log::kStatusbarLogSuccess (i.e. 0): Valid handle
 *         - -1: Invalid handle: Valid flag of handle set to false
 *         - -2: Invalid handle: Handle index out of bounds in
 * `statusbar_registry`
 *         - -3: Invalid handle: Handle IDs don't match between handle struct
 *         - -4: Invalid handle: Handle ID is 0 (i.e. invalid)
 *         - -5: Invalid handle: Errorcode not handled
 * and registry
 *
 * \see _IsValidHandle: Non verbose version of this function
 */
int _IsValidHandleVerbose(const SinkHandle& sink_handle) {
  const int is_valid_handle = _IsValidHandle(sink_handle);
  if (is_valid_handle == -1) {
    LogWrn(kFilename,
            "Invalid handle: Valid flag set to false (idx: %zu, ID: %u)",
            sink_handle.idx, sink_handle.id);
    return -1;
  }

  else if (is_valid_handle == -2) {
    LogWrn(kFilename,
            "Invalid Handle: Handle index %zu out of bounds (max %zu)",
            sink_handle.idx, _sink_registry.size());
    return -2;
  }

  else if (is_valid_handle == -3) {
    LogWrn(kFilename, "Invalid Handle: ID mismatch: handle %u vs registry %u",
            sink_handle.id, _sink_registry[sink_handle.idx]->id);
    return -3;
  }

  else if (is_valid_handle != -4) {
    LogWrn(kFilename, "Invalid Handle: ID is 0 (i.e. invalid)");
    return -4;
  }

  else if (is_valid_handle != kStatusbarLogSuccess) {
    LogWrn(kFilename, "Invalid Handle: Errorcode not handled!");
    return -5;
  }

  return kStatusbarLogSuccess;
}

}  // namespace

int CreateSinkStdout(SinkHandle& sink_handle) {
  const int err = _IsValidHandle(sink_handle);
  if (err == kStatusbarLogSuccess) {
    LogErr(kFilename,
           "Handle already is valid, cannot use it to create a new sink");
    return -1;
  }
  sink_handle.valid = false;
  sink_handle.id = 0;

  if (_sink_registry.size() - _sink_free_handles.size() >= kMaxSinkHandles) {
    LogErr(kFilename,
           "Failed to create sink handle. Maximum number of sink handles (%zu) "
           "reached",
           kMaxSinkHandles);
    return -2;
  }

  std::unique_lock<std::mutex> registry_lock(_sink_registry_mutex,
                                             std::defer_lock);
  std::unique_lock<std::mutex> id_count_lock(_sink_id_count_mutex,
                                             std::defer_lock);
  std::lock(registry_lock, id_count_lock);

  _sink_handle_id_count++;
  if (_sink_handle_id_count == 0) {
    LogWrn(kFilename,
           "Max number of possible sink handle ids reached, looping back to 1");
  }

  if (!_sink_free_handles.empty()) {
    SinkHandle free_handle = _sink_free_handles.back();
    _sink_free_handles.pop_back();
    sink_handle.idx = free_handle.idx;
    // std::mutex
    _sink_registry[sink_handle.idx]->out = &std::cout;
    _sink_registry[sink_handle.idx]->owned_file.reset();
    _sink_registry[sink_handle.idx]->type = kSinkStdout;
    _sink_registry[sink_handle.idx]->fd = fileno(stdout);
    _sink_registry[sink_handle.idx]->id = _sink_handle_id_count;
  } else {
    std::unique_ptr<Sink> new_sink = std::make_unique<Sink>();
    // std::mutex
    new_sink->out = &std::cout;
    new_sink->owned_file.reset();
    new_sink->type = kSinkStdout;
    new_sink->fd = fileno(stdout);
    new_sink->id = _sink_handle_id_count;
    _sink_registry.push_back(std::move(new_sink));
  }
  sink_handle.id = _sink_handle_id_count;
  sink_handle.valid = true;


  return kStatusbarLogSuccess;
}

int CreateSinkFile(SinkHandle& sink_handle, const std::string& path) {
  const int err = _IsValidHandle(sink_handle);
  if (err == kStatusbarLogSuccess) {
    LogErr(kFilename,
           "Handle already is valid, cannot use it to create a new sink");
    return -1;
  }
  sink_handle.valid = false;
  sink_handle.id = 0;

  if (_sink_registry.size() - _sink_free_handles.size() >= kMaxSinkHandles) {
    LogErr(kFilename,
           "Failed to create sink handle. Maximum number of sink handles (%zu) "
           "reached",
           kMaxSinkHandles);
    return -2;
  }

  std::unique_lock<std::mutex> registry_lock(_sink_registry_mutex,
                                             std::defer_lock);
  std::unique_lock<std::mutex> id_count_lock(_sink_id_count_mutex,
                                             std::defer_lock);
  std::lock(registry_lock, id_count_lock);

  _sink_handle_id_count++;
  if (_sink_handle_id_count == 0) {
    LogWrn(kFilename,
           "Max number of possible sink handle ids reached, looping back to 1");
  }

  std::unique_ptr<std::ofstream> f;
  try {
     f = std::make_unique<std::ofstream>(path, std::ios::app);
    if (!f->is_open() || !f->good()) {
      return -3;
    }
  } catch (...) {
    return -4;
  }

  if (!_sink_free_handles.empty()) {
    SinkHandle free_handle = _sink_free_handles.back();
    _sink_free_handles.pop_back();
    sink_handle.idx = free_handle.idx;
    // std::mutex
    _sink_registry[sink_handle.idx]->out = f.get();
    _sink_registry[sink_handle.idx]->owned_file = std::move(f);
    _sink_registry[sink_handle.idx]->type = kSinkFileOwned;
    _sink_registry[sink_handle.idx]->fd = -1;
    _sink_registry[sink_handle.idx]->id = _sink_handle_id_count;
  } else {
    std::unique_ptr<Sink> new_sink = std::make_unique<Sink>();
    // std::mutex
    new_sink->out = f.get();
    new_sink->owned_file = std::move(f);
    new_sink->type = kSinkFileOwned;
    new_sink->fd = -1;
    new_sink->id = _sink_handle_id_count;
    _sink_registry.push_back(std::move(new_sink));
  }
  sink_handle.id = _sink_handle_id_count;
  sink_handle.valid = true;


  return kStatusbarLogSuccess;
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
