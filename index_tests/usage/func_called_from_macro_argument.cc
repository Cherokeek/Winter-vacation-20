#define MACRO_CALL(e) e

bool called(bool a, bool b);

void caller() {
  MACRO_CALL(called(true, true));
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 3787803219955606747,
      "detailed_name": "bool called(bool a, bool b)",
      "qual_name_offset": 5,
      "short_name": "called",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["3:6-3:12|3:1-3:28|1|-1"],
      "derived": [],
      "uses": ["6:14-6:20|16420|-1", "6:14-6:20|64|0"]
    }, {
      "usr": 11404881820527069090,
      "detailed_name": "void caller()",
      "qual_name_offset": 5,
      "short_name": "caller",
      "spell": "5:6-5:12|5:1-7:2|2|-1",
      "bases": [],
      "vars": [],
