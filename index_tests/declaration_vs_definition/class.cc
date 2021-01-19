class Foo;
class Foo;
class Foo {};
class Foo;

/*
// NOTE: Separate decl/definition are not supported for classes. See source
//       for comments.
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [{
      "usr": 15041163540773201510,
      "detailed_name": "class Foo",
      "qual_name_offset": 6,
      "short_name": "Foo",
      "spell": "3:7-3:10|3:1-3:13|2|-1"