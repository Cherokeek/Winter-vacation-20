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
      "detailed_name": "virtual void Foo