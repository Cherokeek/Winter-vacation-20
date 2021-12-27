struct Foo {
  void foo();
};

void usage() {
  Foo* f = nullptr;
  f->foo();
}
/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 6767773193109753523,
      "detailed_name": "void usage()",
      "qual_name_offset": 5,
      "short_name": "usage",
      "spell": "5:6-5:11|5:1-8:2|2|-1",
      "bases": [],
      "vars": [16229832321010999607],
      "callees": ["7:6-7:9|17922201480358737771|3|16420"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 17922201480358737771,
      "detailed_name": "void Foo::foo()",
      "qual_name_offset": 5,
      "short_name": "foo",
      "bases": [],
      "vars": [],
      "callees": [],
      