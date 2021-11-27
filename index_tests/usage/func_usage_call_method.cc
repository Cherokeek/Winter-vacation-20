struct Foo {
  void Used();
};

void user() {
  Foo* f = nullptr;
  f->Used();
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 9376923949268137283,
      "detailed_name": "void user()",
      "qual_name_offset": 5,
      "short_name": "user",
      "spell": "5:6-5:10|5:1-8:2|2|-1",
      "bases": [],
      "vars": [14045150712868309451],
      "callees": ["7:6-7:10|18417145003926999463|3|16420"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      