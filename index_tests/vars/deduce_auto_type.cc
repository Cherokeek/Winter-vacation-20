class Foo {};
void f() {
  auto x = new Foo();
  auto* y = new Foo();
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 880549676430489861,
      "detailed_name": "void f()",
      "qual_name_offset": 5,
      "short_name": "f",
      "spell": "2:6-2:7|2:1-5:2|2|-1",
      "bases": [],
      "vars": [10601729374837386290, 18422884837902130475],
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
      "detailed_name": "class Foo {}",
      "qual_name_offset": 6,
      "short_name": "Foo",
      "spell": "1:7-1:10|1:1-1:13|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "va