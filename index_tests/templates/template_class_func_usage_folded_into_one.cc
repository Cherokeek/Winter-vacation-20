template<typename T>
struct Foo {
  static int foo() {
    return 3;
  }
};

int a = Foo<int>::foo();
int b = Foo<bool>::foo();

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 8340731781048851399,
      "detailed_name": "static int Foo::foo()",
      "qual_name_offset": 11,
      "short_name": "foo",
      "spell": "3:14-3:17|3:3-5:4|1026|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 254,
      "parent_kind": 23,
      "storag