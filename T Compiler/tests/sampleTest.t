class A
{
  int i;
  int f(int j){ out 12; }
}

int main()
{
  A a;

  a = new A();

  a.f(1);
}
