template<typename T>
struct Foo;

using Foo1 = Foo<int>;
typedef Foo<Foo1> Foo2;

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [{
      "usr": 1