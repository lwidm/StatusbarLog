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

#include <ostream>
#include <string>

namespace statusbar_log {
namespace sink {

constexpr unsigned int kMaxSinkHandles = 20;

/**
 * \enum SinkType
 *
 * TODO : write this
*/
typedef enum { kSinkInvalid, kSinkStdout, kSinkFileOwned, kSinkOstreamWrapped} SinkType;

/**
 * \struct SinkHandle
 * \brief Handle to a Sink. Used to interact with the underlying sink
 * struct without directly touching it
 *
 * \see Sink: Underlying sink struct
 */
typedef struct {
  std::size_t idx;  ///< Positional index corresponding to the sinks
                    ///< position in the registry
  unsigned int id;  ///< ID of the sink. Must be unique. Used to verify
                    ///< validity of statusbar
  bool valid;  ///< weather or not this sink is valid (for e.g. false after
               ///< destruction)
} SinkHandle;

/**
 * \brief Initialises a sink that wraps std::cout (does not take ownership) and updates its handle.
 *
 * This function takes an empty SinkHandle struct and creates an associated sink that wraps std::cout (does not take ownership)
 *
 * \param[out] sink_handle Struct to initialize.
 *
 * \warning Don't forget to destroy the sink_handle after use.
 *
 * \return Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) on success, or
 * one of these error/warning codes:
 *         -  statusbar_log::kStatusbarLogSuccess (i.e. 0): Success (no errors)
 *         - -1: Failed to create sink handle (handle already valid)
 *         - -2: Failed to create sink handle (handle registry exceeds
 * maximum element limit)
 *
 * \see Sink: The sink struct.
 * \see _sink_registry: The registry for sink struct in use.
 * \see _sink_free_handles: The registry for free sink handles.
*/
int CreateSinkStdout(SinkHandle& sink_handle);

/**
 * \brief Initialises a sink that opens/owns the given file path (append mode)
 *
 * This function takes an empty SinkHandle struct and creates an associated sink that opens/owns a file path (takes owvernship). It opens the file in append mode.
 *
 * \param[out] sink_handle Struct to initialize.
 * \param[in] path to the file to be opened/created.
 *
 * \warning Don't forget to destroy the sink_handle after use.
 *
 * \return Returns statusbar_log::kStatusbarLogSuccess (i.e. 0) on success, or
 * one of these error/warning codes:
 *         -  statusbar_log::kStatusbarLogSuccess (i.e. 0): Success (no errors)
 *         - -1: Failed to create sink handle (handle already valid)
 *         - -2: Failed to create sink handle (handle registry exceeds
 * maximum element limit)
 *         - -3: Failed to create sink handle (failed in opening file)
 *         - -4: Failed to create sink handle (unknown error in opening file)
 *
 * \see Sink: The sink struct.
 * \see _sink_registry: The registry for sink struct in use.
 * \see _sink_free_handles: The registry for free sink handles.
*/
int CreateSinkFile(SinkHandle& sink_handle, const std::string& path) {

/**
 * \brief Initialize a sink that wraps an existing std::ostream (non-owning).
*/
int SinkInitOstream(SinkHandle& sink, std::ostream& os);

/**
 * \brief Write len bytes (returns number of bytes written or -1 on error).
*/
ssize_t SinkWrite(SinkHandle &s, const char *buf, std::size_t len);

/**
 * Convenience to write a NUL-terminated string (returns 0 on success, -1 on error).
*/
ssize_t SinkWriteStr(SinkHandle &s, const std::string& str);

/**
 * Flush the underlying stream (0 on success, -1 on error).
*/
int SinkFlush(SinkHandle &s);

/**
 * Close/free resources associated with sink (safe to call multiple times).
*/
void SinkClose(SinkHandle &s);

/**
 * Returns true if the underlying stream is a TTY (best-effort).
*/
bool SinkIsTty(SinkHandle &s);

} // namespace sink
} // namespace statusbar_log

#endif  // !STATUSBARLOG_SINK_H_
