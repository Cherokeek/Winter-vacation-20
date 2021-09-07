union vector3 {
  struct { float x, y, z; };
  float v[3];
};

/*
OUTPUT:
{
  "includes": [],
  "skipped_ranges": [],
  "usr2func": [],
  "usr2type": [{
      "usr": 82,
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
      "instances": [3348817847649945564, 4821094820988543895, 15292551660437765731],
      "uses": []
    }, {
      "usr": 1428566502523368801,
      "detailed_name": "anon struct",
      "qual_name_offset": 0,
      "short_name": "anon struct",
      "spell": "2:3-2:9|2:3-2:28|1026|-1",
      "bases": [],
      "funcs": [],
      "types": [],
      "vars": [{
          "L": 3348817847649945564,
          "R": 0
        }, {
          "L": 4821094820988543895,
          "R": 32
        }, {
          "L": 15292551660437765731,
          "R": 64
        }],
      "alias_of": 0,
      "kind": 23,
      "parent_kind": 5,
      "declarations": [],
      "derived": [],
      "instances": [],
      "uses": []
    }, {
      "usr": 17937907487590875128,
      "detailed_name": "union vector3 {}",
      "qual_name_offset": 6,
      "short_name": "vector3",
      "spell": "1:7-1:14|1:1-4:2|2|-1",
      "bases": [],
      "funcs": [],
      "types": [1428566502523368801],
      "vars": [{
          "L": 1963212417280098348,
          "R": 0
        }, {
          "L": 3348817847649945564,
          "R": 0
        }, {
          "L": 4821094820988543895,
          "R": 32
        }, {
          "L": 15292551660437765731,
          "R": 64
        }],
      "alias_of": 0,
      "kind": 5,
      "parent_kind": 0,
      "declarations": [],
      "derived": [],
      "instances": [],
      "uses": []
    }],
  "usr2var": [{
      "usr": 1963212417280098348,
      "detailed_name": "float vector3::v[3]",
      "qual_name_offset": 6,
      "sh