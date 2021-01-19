int foo(int, int);
int foo(int aa,
        int bb);
int foo(int aaa, int bbb);
int foo(int a, int b) { return 0; }

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 2747674671862363334,
      "detailed_name": "int foo(int, int)",
      "qual_name_offset": 4,
      "short_name": "foo",
      "spell": "5:5-5:8|5:1-5:36|2|-1",
      "bases": [],
      "vars": [14555488990109936920, 10963664335057337329],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["1:5-