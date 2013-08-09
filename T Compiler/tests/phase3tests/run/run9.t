class A
{
  int i;
}

class B extends A
{
  int i;
  int j;
}

int main()
{
  A a;
  B b;

  b = new B();
  b.i = 12;
  b.j = 14;

  a = b;
  b = null;
  a.i = 16;

  b = (B) a;

  out a.i + b.i + b.j;

  return 0;
}
