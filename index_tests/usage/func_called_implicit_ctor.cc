struct Wrapper {
  Wrapper(int i);
};

int called() { return 1; }

Wrapper caller() {
  return called();
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 468307235068920063,
      "detailed_name": "int called()",
      "qual_name_offset": 4,
      "short_name": "called",
      "spell": "5:5-5:11|5:1-5:27|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": ["8:10-8:16|16420|-1"]
    }, {
      "usr": 10544127002917214589,
      "detailed_name": "Wrapper::Wrapper(int i)",
      "qual_name_offset": 0,
      "short_name": "Wrapper",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 9,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["2:3-2:10|2:3-2:17|1025|-1"],
      "derived": [],
      "uses": ["8:10-8:16|16676|-1"]
    }, {
      "usr": 11404881820527069090,
      "detailed_name": "Wrapper caller()",
      "qual_name_offset": 8,
      "short_name": "caller",
      "spell": "7:9-7:15|7:1-9:2|2|-1",
      "bases": [],
      "vars": []