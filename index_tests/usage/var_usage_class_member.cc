class Foo {
public:
  int x;
  int y;
};

void accept(int);
void accept(int*);

void foo() {
  Foo f;
  f.x = 3;
  f.x += 5;
  accept(f.x);
  accept(f.x + 20);
  accept(&f.x);
  accept(f.y);
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
      "spell": "10:6-10:9|10:1-18:2|2|-1",
      "bases": [],
      "vars": [14669930844300034456],
      "callees": ["14:3-14:9|17175780305784503374|3|16420", "15:3-15:9|17175780305784503374|3|16420", "16:3-16:9|12086644540399881766|3|16420", "17:3-17:9|17175780305784503374|3|16420"],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 12086644540399881766,
      "detailed_name": "void accept(int *)",
      "qual_name_offset": 5,
      "short_name": "accept",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["8:6-8:12|8:1-8:18|1|-1"],
      "derived": [],
      "uses": ["16:3-16:9|16420|-1"]
    }, {
      "usr": 17175780305784503374,
      "detailed_name": "void accept(int)",
      "qual_name_offset": 5,
      "short_name": "accept",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": ["7:6-7:12|7:1-7:17|1|-1"],
      "derived": [],
      "uses": ["14:3-14:9|16420|-1", "15:3-15:9|16420|-1", "17:3-17:9|16420|-1"]
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
      "instances": [4220150017963593039, 3873837747174060388],
      "uses": []
    }, {
      "usr": 15041163540773201510,
      "detailed_name": "class Foo {}",
      "qual_name_offset": 6,
      "short_name": "Foo",
      "spell": "1:7-1:10|1:1-5:2|2|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [{
          "L": 422015001796