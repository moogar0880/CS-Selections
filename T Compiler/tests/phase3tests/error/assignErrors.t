class A
{
  int i;
}

class B extends A
{
  int i;
}

class C
{
  int i;
}

int main()
{
  A a;
  B b;
  C c;

  a = b;

  a = c;

  b = a;

  b = c;

  c = a;

  c = b;

  a = null;

  a = b = c = null;

  a = b = null;

  b = a = null;
}
