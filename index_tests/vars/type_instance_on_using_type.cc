struct S {};
using F = S;
void Foo() {
  F a;
}

// TODO: Should we also add a usage to |S|?

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func"