class A
{
  int i;
}

class B extends A
{
  int abc;
}

class C extends B
{
  int def;
}

class D extends C
{
  int i;
  int j;
}

int main()
{
  A a;
  D d;
  Object obj;

  d = new D();
  d.i = 12;
  d.j = 14;

  obj = d;
  obj = (Object) obj;

  d = (D) d;

  a = d;
  d = null;
  a.i = 16;

  d = (D) a;

  a = (A) a;

  out a.i + d.i + d.j;

  return 0;
}
