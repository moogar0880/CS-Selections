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

  a = new A();

  b = (B) a;

  return 0;
}
