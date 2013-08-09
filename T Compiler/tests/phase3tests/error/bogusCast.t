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

  a = (Obj) b;

  a = (A) i;

  a = (A) a;

  a = (A) b;

  a = (A) c;

  b = (B) a;

  b = (B) b;

  b = (B) c;

  c = (C) a;

  c = (C) b;

  c = (C) c;
}
