struct Type {
  Type() {}
};

void Make() {
  Type foo0;
  auto foo1 = Type();
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 3957104924306079513,
      "detailed_name": "void Make()",
      "qual_name_offset": 5,
      "short_name": "Make",
      "spell": "5:6-5:10|5:1-8:2|2|-1",
      "bases": [],
      "vars": [449111627548814328, 17097499197730163115],
      "callees": ["6:8-6:12|10530961286677896857|3|16676", "6:8-6:12|10530961286677896857|3|16676", "7:15-7:19|10530961286677896857|3|16676", "7:15-7:19|10530961286677896857|3|16676"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []