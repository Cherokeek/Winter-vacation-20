struct Foo;

void foo(Foo* p0, Foo* p1) {}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 8908726657907936744,
      "detailed_name": "void foo(Foo *p0, Foo *p1)",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "3:6-3:9|3:1-3:30|2|-1",
      "bases": [],
      "vars": [8730439006497971620, 2525014371090380500],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }],
  "usr2type": [{
      "usr": 15041163540773201510,
      "detailed_name": "struct Foo",
      "qual_name_offset": 7,
      "short_name": "Foo",
      "bases": [],
      "funcs":