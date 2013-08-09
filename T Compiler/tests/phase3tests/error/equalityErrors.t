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
  int i;
  A a;
  B b;
  C c;

  i = a == b;

  i = a == c;

  i = b == a;

  i = b == c;

  i = c == a;

  i = c == b;

  i = null == a;

  i = a == null;
}
