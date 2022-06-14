struct ForwardType;
struct ImplementedType {};

void foo(ForwardType* f, ImplementedType a) {}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 1699390678058422036,
      "detailed_name": "void foo(ForwardType *f, ImplementedType a)",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "4:6-4:9|4:1-4:47|2|-1",
      "bases": [],
      "vars": [13058491096576226774, 11055777568039014776],
      "ca