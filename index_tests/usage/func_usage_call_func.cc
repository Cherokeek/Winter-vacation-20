void called() {}
void caller() {
  called();
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 468307235068920063,
      "detailed_name": "void called()",
      "qual_name_offset": 5,
      "short_name": "called",
      "spell": "1:6-1:12|1:1-1:17|2|-1",
      "bases": [],
