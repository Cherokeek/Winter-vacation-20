
struct Foo;

void foo(Foo* f, Foo*);
void foo(Foo* f, Foo*) {}

/*
// TODO: No interesting usage on prototype. But maybe that's ok!
// TODO: We should have the same variable declared for both prototype and
//       declaration. So it should have a usage marker on both. Then we could
//       rename parameters!

OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 8908726657907936744,
      "detailed_name": "void foo(Foo *f, Foo *)",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "4:6-4:9|4:1-4:26|2|-1",
      "bases": [],
      "vars": [13823260660189154978],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["3:6-3:9|3:1-3:23|1|-1"],
      "derived": [],
      "uses": []
    }],
  "usr2type": [{
      "usr": 15041163540773201510,
      "detailed_name": "struct Foo",
      "qual_name_offset": 7,
      "short_name": "Foo",
      "bases": [],
      "funcs": [],
      "types": [],