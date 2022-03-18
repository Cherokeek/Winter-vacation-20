template<typename T, typename B>
class unique_ptr;

struct S1;
struct S2;

#if false
VarDecl f
  TemplateRef unique_ptr
  TemplateRef unique_ptr
  TypeRef struct S1
  TypeRef struct S2
  TypeRef struct S2
#endif
extern unique_ptr<unique_ptr<S1, S2>, S2> f;

#if false
FunctionDecl as_return_type
  TemplateRef unique_ptr
  TemplateRef unique_ptr
  TypeRef struct S1
  TypeRef struct S2
  TypeRef struct S2
  ParmDecl
    TemplateRef unique_ptr
    TypeRef struct S1
    TypeRef struct S2
  CompoundStmt
    ReturnStmt
      UnexposedExpr
        CXXNullPtrLiteralExpr
#endif
unique_ptr<unique_ptr<S1, S2>, S2>* as_return_type(unique_ptr<S1, S2>*) { return nullptr; }

#if false
FunctionDecl no_return_type
  ParmDecl
  CompoundStmt
#endif
void no_return_type(int) {}

#if false
FunctionDecl empty
  CompoundStmt
    DeclStmt
      VarDecl local
        TemplateRef unique_ptr
        TemplateRef unique_ptr
        TypeRef struct S1
        TypeRef struct S2
        TypeRef struct S2
#endif
void empty() {
  unique_ptr<unique_ptr<S1, S2>, S2>* local;
}

#if false
ClassDecl Foo
  CXXMethod foo
    TemplateRef unique_ptr
    TypeRef struct S1
    TypeRef struct S2
#endif
class Foo {
  unique_ptr<S1, S2>* foo();
};

#if false
CXXMethod foo
  TemplateRef unique_ptr
  TypeRef struct S1
  TypeRef struct S2
  TypeRef class Foo
  CompoundStmt
    ReturnStmt
      UnexposedExpr
        CXXNullPtrLiteralExpr
#endif
unique_ptr<S1, S2>* Foo::foo() { return nullptr; }

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": ["7:1-15:1", "17:1-33:1", "35:1-40:1", "42:1-53:1", "57:1-64:1", "68:1-79:1"],
  "usr2func": [{
      "usr": 1246637699196435450,
      "detailed_name": "unique_ptr<unique_ptr<S1, S2>, S2> *as_return_type(unique_ptr<S1, S2> *)",
      "qual_name_offset": 36,
      "short_name": "as_return_type",
      "spell": "33:37-33:51|33:1-33:92|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 13067214284561914253,
      "detailed_name": "void no_return_type(int)",
      "qual_name_offset": 5,
      "short_name": "no_return_type",
      "spell": "40:6-40:20|40:1-40:28|2|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }, {
      "usr": 17922201480358737771,
      "detailed_name": "unique_ptr<S1, S2> *Foo::foo()",
      "qual_name_offset": 20,
      "short_name": "foo",
      "spell": "79:26-79:29|79:1-79:51|1026|-1",
      "bases": [],
      "vars": [],
      "callees": [],
      "kind": 6,
      "parent_kind": 5,
      "storage": 0,
      "declarations": ["65:23-65:26|65:3-65:28|1025|-1"],
      "derived": [],
      "uses": []
    }, {
      "usr": 18320186404467436976,
      "detailed_name": "void empty()",
      "qual_name_offset": 5,
      "short_name": "empty",
      "spell": "53:6-53:11|53:1-55:2|2|-1",
      "bases": [],
      "vars": [500112618220246],
      "callees": [],
      "kind": 12,
      "parent_kind": 0,
      "storage": 0,
      "declarations": [],
      "derived": [],
      "uses": []
    }],
  "usr2type": [{
      "usr": 4310164820010458371,
      "detailed_name": "struct S1",
      "qual_name_offset": 7,
      "short_name": "S1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 0,
      "declarations": ["4:8-4:10|4:1-4:10|1|-1"],
      "derived": [],
      "instances": [],
      "uses": ["15:30-15:32|4|-1", "33:23-33:25|4|-1", "33:63-33:65|4|-1", "54:25-54:27|4|-1", "65:14-65:16|4|-1", "79:12-79:14|4|-1"]
    }, {
      "usr": 12728490517004312484,
      "detailed_name": "struct S2",
      "qual_name_offset": 7,
      "short_name": "S2",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 0,
      "declarations": ["5:8-5:10|5:1-5:10|1|-1"],
      "derived": [],
      "instances": [],
      "uses": ["15:34-15:36|4|-1", "15:39-15:41|4|-1", "33:27-3