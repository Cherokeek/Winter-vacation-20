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
      "alias_of": 