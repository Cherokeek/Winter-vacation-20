
// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "config.hh"
#include "serializer.hh"
#include "utils.hh"

#include <rapidjson/fwd.h>

#include <chrono>
#include <iosfwd>
#include <string>

namespace ccls {
struct RequestId {
  // The client can send the request id as an int or a string. We should output
  // the same format we received.
  enum Type { kNone, kInt, kString };
  Type type = kNone;

  std::string value;

  bool valid() const { return type != kNone; }
};
void reflect(JsonReader &visitor, RequestId &value);
void reflect(JsonWriter &visitor, RequestId &value);

struct InMessage {
  RequestId id;
  std::string method;
  std::unique_ptr<char[]> message;
  std::unique_ptr<rapidjson::Document> document;
  std::chrono::steady_clock::time_point deadline;
  std::string backlog_path;
};

enum class ErrorCode {
  // Defined by JSON RPC
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603,
  serverErrorStart = -32099,
  serverErrorEnd = -32000,
  ServerNotInitialized = -32002,
  UnknownErrorCode = -32001,

  // Defined by the protocol.
  RequestCancelled = -32800,
};

struct ResponseError {
  ErrorCode code;
  std::string message;
};

constexpr char ccls_xref[] = "ccls.xref";
constexpr char window_showMessage[] = "window/showMessage";

struct DocumentUri {
  static DocumentUri fromPath(const std::string &path);

  bool operator==(const DocumentUri &o) const { return raw_uri == o.raw_uri; }
  bool operator<(const DocumentUri &o) const { return raw_uri < o.raw_uri; }

  void setPath(const std::string &path);
  std::string getPath() const;

  std::string raw_uri;
};

struct Position {
  int line = 0;
  int character = 0;
  bool operator==(const Position &o) const {
    return line == o.line && character == o.character;
  }
  bool operator<(const Position &o) const {
    return line != o.line ? line < o.line : character < o.character;
  }
  bool operator<=(const Position &o) const {
    return line != o.line ? line < o.line : character <= o.character;
  }
  std::string toString() const;
};

struct lsRange {
  Position start;
  Position end;
  bool operator==(const lsRange &o) const {
    return start == o.start && end == o.end;
  }
  bool operator<(const lsRange &o) const {
    return !(start == o.start) ? start < o.start : end < o.end;
  }
  bool includes(const lsRange &o) const {
    return start <= o.start && o.end <= end;
  }
  bool intersects(const lsRange &o) const {
    return start < o.end && o.start < end;
  }
};

struct Location {
  DocumentUri uri;
  lsRange range;
  bool operator==(const Location &o) const {
    return uri == o.uri && range == o.range;
  }
  bool operator<(const Location &o) const {
    return !(uri == o.uri) ? uri < o.uri : range < o.range;
  }
};

struct LocationLink {
  std::string targetUri;
  lsRange targetRange;
  lsRange targetSelectionRange;
  explicit operator bool() const { return targetUri.size(); }
  explicit operator Location() && {
    return {DocumentUri{std::move(targetUri)}, targetSelectionRange};
  }
  bool operator==(const LocationLink &o) const {
    return targetUri == o.targetUri &&
           targetSelectionRange == o.targetSelectionRange;
  }
  bool operator<(const LocationLink &o) const {
    return !(targetUri == o.targetUri)
               ? targetUri < o.targetUri
               : targetSelectionRange < o.targetSelectionRange;
  }
};

enum class SymbolKind : uint8_t {
  Unknown = 0,

  File = 1,
  Module = 2,
  Namespace = 3,