struct ForwardType;
void foo(ForwardType*) {}
/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 15327735280790448926,
      "detailed_name": "void foo(ForwardType *)",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "2:6-2:9|2:1-2:26|2|-1",
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
      "usr": 13749354388332789217,
      "detailed_name": "struct ForwardTyp