class Foo {
  void declonly();
  virtual void purevirtual() = 0;
  void def();
};

void Foo::def() {}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 4012226004228259562,
      "detailed_name": "void Foo::declonly()",
      "qual_name_offset": 5,
      "short_name": "declonly",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 6,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["2:8-2:16|2:3-2:18|1025|-1"],
      "derived": [],
      "uses": []
    }, {
      "usr": 10939323144126021546,
      "detailed_name": "virtual void Foo::purevirtual() = 0",
      "qual_name_offset": 13,
      "short_name": "purevirtual",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 6,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["3:16-3:27|3:3-3:33|1089|-1"],
      "derived": [],
      "uses": []
    }, {
      "usr": 15416083548883122431,
      "detailed_name": "void Foo::def()",
      "qual_name_offset": 5,
      "short_name": "def",
      "spell": "7:11-7:14|7:1-7:19|1026|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 6,
      "parent_kind": 5,
      "storage": 