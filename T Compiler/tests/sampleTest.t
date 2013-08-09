class A
{
  int i;
}

int main()
{
  A a;

  a = new A();

  a.i = 42;

  out a.i;

  delete a;

  return 0;
}