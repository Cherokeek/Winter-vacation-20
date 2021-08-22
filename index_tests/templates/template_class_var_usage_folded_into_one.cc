
template<typename T>
struct Foo {
  static constexpr int var = 3;
};

int a = Foo<int>::var;
int b = Foo<bool>::var;
