struct S {};
using F = S;
void Foo() {
  F a;
}

// TODO: Should we also add a usage to |S|?

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 4654328188330986029,
      "detailed_name": "void Foo()",
      "qual_name_offset": 5,
      "short_name": "Foo",
      "spell": "3:6-3:9|3:1-5:2|2|-1",
      "bases": [],
      "vars": [6975456769752895964],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }],
  "usr2type": [{
      "usr": 4750332761459066907,
      "detailed_name": "struct S {}",
      "qual_name_offset": 7,
      "short_name": "S",
      "spell": "1:8-1:9|1:1-1:12|2|-1",
      "bases": [],
      "f