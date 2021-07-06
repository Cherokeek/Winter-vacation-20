@interface AClass
  + (void)test;
  - (void)anInstanceMethod;
  @property (nonatomic) int aProp;
@end

@implementation AClass
+ (void)test {}
- (void)anInstanceMethod {}
@end

int main(void)
{
  AClass *instance = [AClass init];
  [instance anInstanceMethod];
  instance.aProp = 12;
}

/*
OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [{
      "id": 0,
      "usr": 11832280568361305387,
      "detailed_name": "AClass",
      "short_name": "AClass",
      "kind": 7,
      "spell": "7:17-7:23|-1|1|2",
      "extent": "7:1-10:2|-1|1|0",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [],
      "vars": [],
      "instances": [2],
      "uses": ["14:3-14:9|-1|1|4", "14:23-14:29|-1|1|4"]
    }, {
      "id": 1,
      "usr": 17,
      "detailed_name": "",
      "short_name": "",
      "kind": 0,
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [],
      "vars": [],
      "instances": [0, 1],
      "uses": []
    }],
  "funcs": [{
      "id": 0,
      "usr": 12775970426728664910,
      "detailed_name": "AClass::test",
      "short_name": "test",
      "kind": 17,
      "storage": 0,
      "declarations": [{
          "spelling": "2:11-2:15",
          "extent": "2:3-2:16",
          "content": "+ (void)test;",
          "param_spellings": []
        }],
      "spell": "8:9-8:13|-1|1|2",
      "extent": "8:1-8:16|-1|1|0",
      "base": [],
      "derived": [],
      "locals": [],
      "uses": [],
      "callees": []
    }, {
      "id": 1,
      "usr": 4096877434426330804,
      "detailed_name": "AClass::anInstanceMethod",
      "short_name": "anInstanceMethod",
      "kind": 16,
      "storage": 0,
      "declarations": [{
          "spelling": "3:11-3:27",
          "extent": "3:3-3:28",
          "content": "- (void)anInstanceMethod;",
          "param_spellings": []
        }],
      "spell": "9:9-9:25|-1|1|2",
      "extent": "9:1-9:28|-1|1|0",
      "base": [],
      "derived": [],
      "locals": [],
      "uses": ["15:13-15:29|4|3|64"],
      "callees": []
    }, {
      "id": 2,
      "usr": 12774569141855220778,
      "detailed_name": "AClass::aProp",
      "short_name": "aProp",
      "kind": 16,
      "storage": 0,
      "declarations": [{
          "spelling": "0:0-0:0",
          "extent": "4:29-4:34",
          "content": "aProp",
          "param_spellings": []
        }],
      "extent": "4:29-4:34|-1|1|0",
      "base": [],
  