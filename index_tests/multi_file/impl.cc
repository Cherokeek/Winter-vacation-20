#include "header.h"

void Impl() {
  Foo1<int>();
}

/*
OUTPUT: header.h
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 11650481237659640387,
      "detailed_name": "void Foo1()",
      "qual_name_offset": 5,
      "short_name": "Foo1",
      "spell": "10:6-10:10|10:1-10:15|2|-1",
      "bas