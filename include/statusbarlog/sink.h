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

// -- statusbarlog/include/statusbarlog/sink.h

#ifndef STATUSBARLOG_SINK_H_
#define STATUSBARLOG_SINK_H_

#include <fstream>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>

namespace statusbar_log {
namespace sink {

/**
 * \enum SinkType
 *
 * TODO : write this
*/
typedef enum { kSinkInvalid, kSinkStdout, kSinkFileOwned, kSinkOstreamWrapped} SinkType;

/**
 *
 *
 * TODO : write this
*/
typedef struct {
  SinkType type;
  std::ostream* out;
  std::unique_ptr<std::ofstream> owned_file;
  std::mutex mutex;
  int fd;
} Sink;

/**
 * \brief Initialises a sink that wraps std::cout (does not take ownership).
*/
int SinkInitStdout(Sink& sink);

/**
 * \brief Initialises a sink that opens/owns the given file path (append mode).
*/
int SinkInitFile(Sink& sink, const std::string& path);

/**
 * \brief Initialize a sink that wraps an existing std::ostream (non-owning).
*/
int SinkInitOstream(Sink& sink, std::ostream& os);

/**
 * \brief Write len bytes (returns number of bytes written or -1 on error).
*/
ssize_t SinkWrite(Sink &s, const char *buf, std::size_t len);

/**
 * Convenience to write a NUL-terminated string (returns 0 on success, -1 on error).
*/
ssize_t SinkWriteStr(Sink &s, const std::string& str);

/**
 * Flush the underlying stream (0 on success, -1 on error).
*/
int SinkFlush(Sink &s);

/**
 * Close/free resources associated with sink (safe to call multiple times).
*/
void SinkClose(Sink &s);

/**
 * Returns true if the underlying stream is a TTY (best-effort).
*/
bool SinkIsTty(Sink &s);

} // namespace sink
} // namespace statusbar_log

#endif  // !STATUSBARLOG_SINK_H_
