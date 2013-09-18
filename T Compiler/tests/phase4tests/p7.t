class B extends A {
  int f(int i, int j)
  {
    return i + j;
  }
}

int main()
{
  A a;
  B b;

  a = new A();
  b = new B();

  out b.f(19) + 14;
  out b.f(a) + 13;
  out b.f(b) + 13;
  out b.f(7, 8);
}

class A {
  int f(int i) { return 1; }
  int f(A a) { return 2; }
}

