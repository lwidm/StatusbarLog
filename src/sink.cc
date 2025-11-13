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

const std::string kFilename = "statusbarlog.cc";

namespace statusbar_log {
namespace sink {

namespace {

/**
 * \struct Sink
 *
 * \brief Sinkt struct contains both outputstreams and filestream as well as
 * identifier to validate associated handles and a mutex to make sure the sink
 * doesn't suffer from race conditions.
 *
 * \see SinkType: All possilbe sink types
 * \see SinkHandle: The handle to the sink
 */
typedef struct {
  std::mutex mutex;
  std::ostream* out;  ///< actual output stream used for printing
  std::unique_ptr<std::ofstream>
      owned_file;  ///< Some sinks need to own a file (nullptr otherwise)
  SinkType type;   ///< The sinks type
  int fd;  ///< File descriptor, used for differenciating between cout and cerr
           ///< (-1 if not applicable)
  unsigned int id;  ///< id of the struct, used for validating handles
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

/**
 * \brief Checks if a sink handle can be used for creating a new sink.
 *
 * This functions checks if the input handle is valid. If so the handle can not
 * be used for sink creation (as it already corresponds to another sink) and
 * returns a negative value. If the maximum number of sinks is reached this
 * function also returns a negative number.
 *
 * \param[in,out] sink_handle The handle to be checked for sink creation
 *
 * \return Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) on success, or
 * one of these error/warning codes:
 *         -  statusbar_log::kStatusbarLogSuccess (i.e. 0): Success (no errors)
 *         - -1: Failed to create sink handle (handle already valid)
 *         - -2: Failed to create sink handle (handle registry exceeds
 */
int _ValidateSinkCreation(SinkHandle& sink_handle) {
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

  return kStatusbarLogSuccess;
}

/**
* \brief Flush the sink
*
* Function tries to flush a sink. Returns kStatusbarLogSuccess on success,
* otherwise a negative integer
*
*\returns Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) on success, or
* one of these error/warnings codes:
*         - statusbar_log::kStatusbarLogSuccess (i.e. 0): Successfully flushed
* sink.
*         - -1: Failed: Sink ostream not functional.
*         - -2: Failed: Sink ostream became not functional after flushing.
*/
int FlushSink(std::unique_ptr<Sink>& sink) {
  std::lock_guard<std::mutex> lk(sink->mutex);
  if (sink->fd >= 0) {
    return 0;
  }
  if (!sink->out->good()) return -1;
  sink->out->flush();
  return sink->out->good() ? 0 : -2;
}

}  // namespace

int CreateSinkStdout(SinkHandle& sink_handle) {
  const int err = _ValidateSinkCreation(sink_handle);
  if (err != kStatusbarLogSuccess) {
    return err;
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
  const int err = _ValidateSinkCreation(sink_handle);
  if (err != kStatusbarLogSuccess) {
    return err;
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

int CreateSinkOstream(SinkHandle& sink_handle, std::ostream& os) {
  const int err = _ValidateSinkCreation(sink_handle);
  if (err != kStatusbarLogSuccess) {
    return err;
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

  int fd;
  if (&os == &std::cout) {
    fd = fileno(stdout);
  } else if (&os == &std::cerr) {
    fd = fileno(stderr);
  } else {
    fd = -1;
  }

  if (!_sink_free_handles.empty()) {
    SinkHandle free_handle = _sink_free_handles.back();
    _sink_free_handles.pop_back();
    sink_handle.idx = free_handle.idx;
    // std::mutex
    _sink_registry[sink_handle.idx]->out = &os;
    _sink_registry[sink_handle.idx]->owned_file.reset();
    _sink_registry[sink_handle.idx]->type = kSinkOstreamWrapped;
    _sink_registry[sink_handle.idx]->fd = fd;
    _sink_registry[sink_handle.idx]->id = _sink_handle_id_count;
  } else {
    std::unique_ptr<Sink> new_sink = std::make_unique<Sink>();
    // std::mutex
    new_sink->out = &os;
    new_sink->owned_file.reset();
    new_sink->type = kSinkOstreamWrapped;
    new_sink->fd = fd;
    new_sink->id = _sink_handle_id_count;
    _sink_registry.push_back(std::move(new_sink));
  }
  sink_handle.id = _sink_handle_id_count;
  sink_handle.valid = true;

  return kStatusbarLogSuccess;
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

int DestroySinkHandle(SinkHandle& sink_handle) {
  int err = _IsValidHandleVerbose(sink_handle);
  if (err != kStatusbarLogSuccess) {
    LogErr(kFilename, "Failed to destory statusbar_handle!");
    return err;
  }

  std::unique_lock<std::mutex> sink_lock;
  GetUniqueLock(sink_handle, sink_lock);
  std::unique_lock<std::mutex> registry_lock(_sink_registry_mutex,
                                             std::defer_lock);
  std::lock(sink_lock, registry_lock);

  std::unique_ptr<Sink> target = std::move(_sink_registry[sink_handle.idx]);

  FlushSink(target);

  target->out = nullptr;
  if (target->owned_file) {
    target->owned_file->close();
    if (!target->owned_file->good()) {
      // TODO: Error message (not sure if it will work here)
      return -6;
    }
    target->owned_file.reset();
  }
  target->type = kSinkInvalid;
  target->fd = -1;
  target->id = 0;

  _sink_registry[sink_handle.idx] = std::move(target);

  sink_handle.valid = false;
  sink_handle.id = 0;
  _sink_free_handles.push_back(sink_handle);

  return kStatusbarLogSuccess;
}

ssize_t SinkWriteStr(Sink& sink, const std::string& str) {
  return SinkWrite(sink, str.c_str(), str.size());
}

bool SinkIsTty(Sink& sink) {
  if (sink.fd >= 0) {
    return ::isatty(sink.fd) != 0;
  }
  if (sink.out == &std::cout) return ::isatty(fileno(stdout));
  if (sink.out == &std::cerr) return ::isatty(fileno(stderr));
  return false;
}

int GetUniqueLock(const SinkHandle& sink_handle,
                  std::unique_lock<std::mutex>& lock) {
  int err = _IsValidHandleVerbose(sink_handle);
  if (err != kStatusbarLogSuccess) return err;
  std::lock_guard<std::mutex> lx(_sink_registry_mutex);
  lock = std::unique_lock<std::mutex>(_sink_registry[sink_handle.idx]->mutex,
                                      std::defer_lock);
  return 0;
}

int GetSinkType(const SinkHandle& sink_handle, SinkType& sink_type) {
  int err = _IsValidHandleVerbose(sink_handle);
  if (err != kStatusbarLogSuccess) return err;
  std::lock_guard<std::mutex> lx(_sink_registry_mutex);
  sink_type = _sink_registry[sink_handle.idx]->type;
  return 0;
}

int FlushSinkHandle(const SinkHandle& sink_handle) {
  int err = _IsValidHandleVerbose(sink_handle);
  if (err != kStatusbarLogSuccess) return err;
  err = FlushSink(_sink_registry[sink_handle.idx]);
  if (err != kStatusbarLogSuccess) {
    return err+5;
  }
  return kStatusbarLogSuccess;
}

}  // namespace sink
}  // namespace statusbar_log
