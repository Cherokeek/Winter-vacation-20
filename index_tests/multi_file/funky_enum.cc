enum Foo {
#include "funky_enum.h"
};

/*
// TODO: In the future try to have better support for types defined across
// multiple files.

OUTPUT: funky_enum.h
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [],
  "usr2var": [{
      "usr": 439339022761937396,
      "detailed_name": "A",
      "qual_name_offset": 0,
      "short_name": "A",
      "hover": "A = 0",
      "comments": "This file cannot be built directory. It is included in an enum definition of\nanother file.",
      "spell": "4:1-4:2|4:1-4:2|1026|-1",
      "type": 16985894625255407295,
      "kind": 22,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "uses": []
    }, {
      "usr": 8524995777615948802,
      "detailed_name": "C",
      "qual_name_offset": 0,
      "short_name": "C",
      "h