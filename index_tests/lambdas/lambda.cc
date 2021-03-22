void foo() {
  int x;

  auto dosomething = [&x](int y) {
    ++x;
    ++y;
  };

  dosomething(1);
  dosomething(1);
  dosomething(1);
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 4259594751088586730,
      "detailed_name": "void foo()",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "1:6-1:9|1:1-12:2|2|-1",
      "bases": [],
      "vars": [12666114896600231317, 2981279427664991319],
      "callees": ["9:14-9:15|17926497908620168464|3|16420", "10:14-10:15|17926497908620168464|3|16420", "11:14-11:15|17926497908620168464|3|16420"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 17926497908620168464,
      "detailed_name": "inline void foo()::(anon class)::operator()(int y) const",
      "qual_name_offset": 12,
      "short_name": "operator()",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 6,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": ["9:14-9:15|16420|-1", "10:14-10:15|16420|-1", "11:14-11:15|16420|-1"]
    }],
  "usr2type": [{
      "usr": 53,
      "detailed_name": "",
      "qual_name_offset": 0,
      "short_name": "",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of