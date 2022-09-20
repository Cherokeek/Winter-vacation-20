void called(int a);

int gen();

struct Foo {
  static int static_var;
  int field_var;
};

int Foo::static_var = 0;

void foo() {
  int a = 5;
  called(a + gen() + Foo().field_var + Foo::static_var);
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
      "spell": "12:6-12:9|12:1-15:2|2|-1",
      "bases": [],
      "vars": [8039186520399841081],
      "callees": ["14:3-14:9|18319417758892371313|3|16420", "14:14-14:17|11404602816585117695|3|16420"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 11404602816585117695,
      "detailed_name": "int gen()",
      "qual_name_offset": 4,
      "short_name": "gen",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["3:5-3:8|3:1-3:10|1|-1"],
      "derived": [],
      "uses": ["14:14-14:17|16420|-1"]
    }, {
      "usr": 18319417758892371313,
      "detailed_name": "void called(int a)",
      "qual_name_offset": 5,
      "short_name": "called",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["1:6-1:12|1:1-1:19|1|-1"],
      "derived": [],
      "uses": ["14:3-14:9|16420|-1"]
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
      "alias_of": 0,
      "kind": 0,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [11489549839875479478, 9648311402855509901, 11489549839875479478, 8039186520399841081],
      "uses": []
    }, {
      "usr": 15041163540773201510,
      "detailed_name": "struct Foo {}",
      "qual_name_offset": 7,
      "short_name": "Foo",
      "spell": "5:8-5:11|5:1-8:2|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [{
          "L": 9648311402855509901,
        