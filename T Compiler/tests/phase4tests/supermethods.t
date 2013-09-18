class A
{
  int i;
  int f(){ return 1; }
  int g()
  {
    out f();
  }
}

class B extends A
{
  int h()
  {
    out super.f();
  }
}

int main()
{
  B b;

  b = new B();

  b.h();

  out 42;
}
