namespace ns {
  enum VarType {};

  template<typename _>
  struct Holder {
    static constexpr VarType static_var = (VarType)0x0;
  };

  template<typename _>
  const typename VarType Holder<_>::static_var;


  int Foo = Holder<int>::static_var;
  int Foo2 = Holder<int>::static_var;
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
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
      "instances": [12898699035586282159, 9008550860229740818],
      "uses": []
    }, {
      "usr": 1532099849728741556,
      "detailed_name": "enum ns::VarType {}",
      "qual_name_offset": 5,
      "short_name": "VarType",
      "spell": "2:8-2:15|2:3-2:18|1026|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [],
      "alias_of": 0,
      "kind": 10,
      "parent_kind": 3,
      "declarations": [],
      "derived": [],
      "instances": [4731849186641714451, 4731849186641714451],
      "uses": ["6:22-6:29|4|-1", "6:44-6:51|4|-1", "10:18-10:25|4|-1"]
    }, {
      "usr": 11072669167287398027,
      "detailed_name": "namespace ns {}",
      "qual_name_offset": 10,
      "short_name": "ns",
      "bases": [],
      "funcs": [],
      "types": [1532099849728741556, 12688716854043726585],
      "vars": [{
          "L": 12898699035586282159,
          "R": -1
        }, {
          "L": 9008550860229740818,
          "R": -1
        }],
      "alias_of": 0,
      "kind": 3,
      "parent_kind": 0,
      "declarations": ["1:11-1:13|1:1-15:2|1|-1"],
      "derived": [],
      "instances": [],
      "uses": []
    }, {
      "usr": 12688716854043726585,
      "detailed_name": "struct ns: