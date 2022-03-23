template<typename T>
class unique_ptr {};

struct S;

static unique_ptr<S> foo;

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [{
      "usr": 3286534761799572592,
      "detailed_name": "class unique_ptr {}",
      "qual_name_offset": 6,
      "short_name": "unique_ptr",
      "spell": "2:7-2:17|2:1-2:20|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 5,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [3398408600781120939],
      "uses": ["6:8-6:18|4|-1"]
    }, {
      "usr": 4750332761459066907,
      "detailed_name": "struct S",
      "qual_name_offset": 7,
      "short_name": "S",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 0,
      "declarations": ["4:8-4:9|4:1-4:9|1|-1"],
      "derived": [],
      "instances": [],
      "uses": ["6:19-6:20|4|-1"]
    }],
  