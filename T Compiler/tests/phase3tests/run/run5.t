int main()
{
  D d;

  d = new D();
  d.c = new C();
  d.c.b = new B();
  d.c.b.a = new A();
  d.c.b.a.i = 42;

  out d.c.b.a.i = 42;
}

class D
{
  C c;
}

class C
{
  B b;
}

class B
{
  A a;
}

class A
{
  int i;
}

