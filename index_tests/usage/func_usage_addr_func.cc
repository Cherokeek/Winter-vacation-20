void consume(void (*)()) {}

void used() {}

void user() {
  void (*x)() = &used;
  consume(&used);
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 5264867802674151787,
      "detailed_name": "void used()",
      "qual_name_offset": 5,
      "short_name": "used",
      "spell": "3:6-3:10|3:1-3:15|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "der