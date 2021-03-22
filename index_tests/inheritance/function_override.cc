class Root {
  virtual void foo();
};
class Derived : public Root {
  void foo() override {}
};

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [{
      "usr": 6666242542855173890,
      "detailed_name": "void Derived::foo()",
      "qual_name_offset": 5,
      "short_name": "foo",
      "spell": "5:8-5:11|5:3-5:25|5186|-1",
      "bases": [9948027785633571339],
      "vars"