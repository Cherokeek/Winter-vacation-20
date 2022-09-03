struct Foo;
using Foo1 = Foo*;
typedef Foo Foo2;
using Foo3 = Foo1;
using Foo4 = int;

void accept(Foo*) {}
void accept1(Foo1*) {}
void accept2(Foo2*) {}
void accept3(Foo3*) {}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 558620830317390922,
      "detailed_name": "void accept1(Foo1 *)",
      "qual_name_offset": 5,
      "short_name": "accept1",
      "spell": "8:6-8:13|8:1-8:23|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 9119341505144503905,
      "detailed_name": "void accept(Foo *)",
      "qual_name_offset": 5,
      "short_name": "accept",
      "spell": "7:6-7:12|7:1-7:21|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 10523262907746124479,
      "detailed_name": "void accept2(Foo2 *)",
      "qual_name_offset": 5,
      "short_name": "accept2",
      "spell": "9:6-9:13|9:1-9:23|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 1