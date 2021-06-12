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
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }],
  "usr2type": [{
      "usr": 53,
      "detailed_name": "",
      "qual_name_offset": 0,
      "short_name": "",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 0,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [2638219001294786365, 8395885290297540138],
      "uses": []
    }, {
      "usr": 529393482671181129,
      "detailed_name": "struct Foo2 {}",
      "qual_name_offset": 7,
      "short_name": "Foo2",
      "spell": "13:8-13:12|13:1-13:15|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [],
      "uses": []
    }, {
      "usr": 619345544228965342,
      "detailed_name": "using Foo0 = SameFileDerived",
      "qual_name_offset": 6,
      "short_name": "Foo0",
      "spell": "7:7-7:11|7:1-7:29|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 16750616846959666305,
      "kind": 252,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [],
      "uses": []
    }, {
      "usr": 4481210672785600703,
      "detailed_name": "enum Foo3 {}",
      "qual_name_offset": 5,
      "short_name": "Foo3",
      "spell": "15:6-15:10|15:1-15:22|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 10,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [],
      "uses": []
    }, {
      "usr": 8420119006782424779,
      "detailed_name": "struct Base {}",
      "qual_name_offset": 7,
      "short_name": "Base",
      "spell": "3:8-3:12|3:1-3:15|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 0,
      "declarations": [],
      "derived": [16750616846959666305],
      "instances": [],
      "uses": ["5:26-5:30|2052|-1"]
    }, {
      "usr": 16750616846959666305,
      "detailed_name": "struct SameFileDerived : Base {}",
      "qual_name_offset": 7,
      "short_name": "SameFileDerived",
      "spell": "5:8-5:23|5:1-5:33|2|-1",
 