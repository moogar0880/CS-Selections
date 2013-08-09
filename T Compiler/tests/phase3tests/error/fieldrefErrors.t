class A
{
  int i;
}

class B extends A
{
  int j;
}

class C
{
  int k;
}

int main()
{
  A a;
  B b;
  C c;
  int i;

  out a.j;

  out null.i;

  out i.k;

  out a.i.j;

  out 17.i;

  out (a = b).i;

  out b.i;
}
