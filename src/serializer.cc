
// Copyright 2017-2020 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "serializer.hh"

#include "filesystem.hh"
#include "indexer.hh"
#include "log.hh"
#include "message_handler.hh"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <llvm/ADT/CachedHashString.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Allocator.h>

#include <mutex>
#include <stdexcept>

using namespace llvm;

bool gTestOutputMode = false;

namespace ccls {

void JsonReader::iterArray(llvm::function_ref<void()> fn) {
  if (!m->IsArray())
    throw std::invalid_argument("array");
  // Use "0" to indicate any element for now.
  path_.push_back("0");
  for (auto &entry : m->GetArray()) {
    auto saved = m;
    m = &entry;
    fn();
    m = saved;
  }
  path_.pop_back();
}
void JsonReader::member(const char *name, llvm::function_ref<void()> fn) {
  path_.push_back(name);
  auto it = m->FindMember(name);
  if (it != m->MemberEnd()) {
    auto saved = m;
    m = &it->value;
    fn();
    m = saved;
  }
  path_.pop_back();
}
bool JsonReader::isNull() { return m->IsNull(); }
std::string JsonReader::getString() { return m->GetString(); }
std::string JsonReader::getPath() const {
  std::string ret;
  for (auto &t : path_)
    if (t[0] == '0') {
      ret += '[';
      ret += t;
      ret += ']';
    } else {
      ret += '/';
      ret += t;
    }
  return ret;
}

void JsonWriter::startArray() { m->StartArray(); }
void JsonWriter::endArray() { m->EndArray(); }
void JsonWriter::startObject() { m->StartObject(); }
void JsonWriter::endObject() { m->EndObject(); }
void JsonWriter::key(const char *name) { m->Key(name); }
void JsonWriter::null_() { m->Null(); }
void JsonWriter::int64(int64_t v) { m->Int64(v); }
void JsonWriter::string(const char *s) { m->String(s); }
void JsonWriter::string(const char *s, size_t len) { m->String(s, len); }

// clang-format off
void reflect(JsonReader &vis, bool &v              ) { if (!vis.m->IsBool())   throw std::invalid_argument("bool");               v = vis.m->GetBool(); }
void reflect(JsonReader &vis, unsigned char &v     ) { if (!vis.m->IsInt())    throw std::invalid_argument("uint8_t");            v = (uint8_t)vis.m->GetInt(); }
void reflect(JsonReader &vis, short &v             ) { if (!vis.m->IsInt())    throw std::invalid_argument("short");              v = (short)vis.m->GetInt(); }
void reflect(JsonReader &vis, unsigned short &v    ) { if (!vis.m->IsInt())    throw std::invalid_argument("unsigned short");     v = (unsigned short)vis.m->GetInt(); }
void reflect(JsonReader &vis, int &v               ) { if (!vis.m->IsInt())    throw std::invalid_argument("int");                v = vis.m->GetInt(); }
void reflect(JsonReader &vis, unsigned &v          ) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned");           v = (unsigned)vis.m->GetUint64(); }
void reflect(JsonReader &vis, long &v              ) { if (!vis.m->IsInt64())  throw std::invalid_argument("long");               v = (long)vis.m->GetInt64(); }
void reflect(JsonReader &vis, unsigned long &v     ) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned long");      v = (unsigned long)vis.m->GetUint64(); }
void reflect(JsonReader &vis, long long &v         ) { if (!vis.m->IsInt64())  throw std::invalid_argument("long long");          v = vis.m->GetInt64(); }
void reflect(JsonReader &vis, unsigned long long &v) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned long long"); v = vis.m->GetUint64(); }
void reflect(JsonReader &vis, double &v            ) { if (!vis.m->IsDouble()) throw std::invalid_argument("double");             v = vis.m->GetDouble(); }
void reflect(JsonReader &vis, const char *&v       ) { if (!vis.m->IsString()) throw std::invalid_argument("string");             v = intern(vis.getString()); }
void reflect(JsonReader &vis, std::string &v       ) { if (!vis.m->IsString()) throw std::invalid_argument("string");             v = vis.getString(); }

void reflect(JsonWriter &vis, bool &v              ) { vis.m->Bool(v); }
void reflect(JsonWriter &vis, unsigned char &v     ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, short &v             ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, unsigned short &v    ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, int &v               ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, unsigned &v          ) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, long &v              ) { vis.m->Int64(v); }
void reflect(JsonWriter &vis, unsigned long &v     ) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, long long &v         ) { vis.m->Int64(v); }
void reflect(JsonWriter &vis, unsigned long long &v) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, double &v            ) { vis.m->Double(v); }
void reflect(JsonWriter &vis, const char *&v       ) { vis.string(v); }
void reflect(JsonWriter &vis, std::string &v       ) { vis.string(v.c_str(), v.size()); }

void reflect(BinaryReader &vis, bool &v              ) { v = vis.get<bool>(); }
void reflect(BinaryReader &vis, unsigned char &v     ) { v = vis.get<unsigned char>(); }
void reflect(BinaryReader &vis, short &v             ) { v = (short)vis.varInt(); }
void reflect(BinaryReader &vis, unsigned short &v    ) { v = (unsigned short)vis.varUInt(); }
void reflect(BinaryReader &vis, int &v               ) { v = (int)vis.varInt(); }
void reflect(BinaryReader &vis, unsigned &v          ) { v = (unsigned)vis.varUInt(); }
void reflect(BinaryReader &vis, long &v              ) { v = (long)vis.varInt(); }
void reflect(BinaryReader &vis, unsigned long &v     ) { v = (unsigned long)vis.varUInt(); }
void reflect(BinaryReader &vis, long long &v         ) { v = vis.varInt(); }
void reflect(BinaryReader &vis, unsigned long long &v) { v = vis.varUInt(); }
void reflect(BinaryReader &vis, double &v            ) { v = vis.get<double>(); }
void reflect(BinaryReader &vis, const char *&v       ) { v = intern(vis.getString()); }
void reflect(BinaryReader &vis, std::string &v       ) { v = vis.getString(); }

void reflect(BinaryWriter &vis, bool &v              ) { vis.pack(v); }
void reflect(BinaryWriter &vis, unsigned char &v     ) { vis.pack(v); }
void reflect(BinaryWriter &vis, short &v             ) { vis.varInt(v); }
void reflect(BinaryWriter &vis, unsigned short &v    ) { vis.varUInt(v); }
void reflect(BinaryWriter &vis, int &v               ) { vis.varInt(v); }
void reflect(BinaryWriter &vis, unsigned &v          ) { vis.varUInt(v); }
void reflect(BinaryWriter &vis, long &v              ) { vis.varInt(v); }
void reflect(BinaryWriter &vis, unsigned long &v     ) { vis.varUInt(v); }
void reflect(BinaryWriter &vis, long long &v         ) { vis.varInt(v); }
void reflect(BinaryWriter &vis, unsigned long long &v) { vis.varUInt(v); }
void reflect(BinaryWriter &vis, double &v            ) { vis.pack(v); }
void reflect(BinaryWriter &vis, const char *&v       ) { vis.string(v); }
void reflect(BinaryWriter &vis, std::string &v       ) { vis.string(v.c_str(), v.size()); }
// clang-format on

void reflect(JsonWriter &vis, std::string_view &data) {
  if (data.empty())
    vis.string("");
  else
    vis.string(&data[0], (rapidjson::SizeType)data.size());
}

void reflect(JsonReader &vis, JsonNull &v) {}
void reflect(JsonWriter &vis, JsonNull &v) { vis.m->Null(); }

template <typename V>
void reflect(JsonReader &vis, std::unordered_map<Usr, V> &v) {
  vis.iterArray([&]() {
    V val;
    reflect(vis, val);
    v[val.usr] = std::move(val);
  });
}
template <typename V>
void reflect(JsonWriter &vis, std::unordered_map<Usr, V> &v) {
  // Determinism
  std::vector<std::pair<uint64_t, V>> xs(v.begin(), v.end());
  std::sort(xs.begin(), xs.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });
  vis.startArray();
  for (auto &it : xs)
    reflect(vis, it.second);
  vis.endArray();
}
template <typename V>
void reflect(BinaryReader &vis, std::unordered_map<Usr, V> &v) {
  for (auto n = vis.varUInt(); n; n--) {
    V val;
    reflect(vis, val);
    v[val.usr] = std::move(val);
  }
}
template <typename V>
void reflect(BinaryWriter &vis, std::unordered_map<Usr, V> &v) {
  vis.varUInt(v.size());
  for (auto &it : v)
    reflect(vis, it.second);
}
