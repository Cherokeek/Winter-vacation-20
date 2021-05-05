enum Foo {
#include "funky_enum.h"
};

/*
// TODO: In the future try to have better support for types defined across
// multiple files.

OUTPUT: funky_enum.h
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [],
  "