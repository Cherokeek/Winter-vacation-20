class Parent {};
class Derived : public Parent {};

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [{
      "usr": 3866412049634585509,
      "detailed_name": "class Parent {}",
      "qual_name_offset": 6,
      "short_name": "Parent",
      "spell": "1:7-1:13|1:1-1:16|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 5,
 