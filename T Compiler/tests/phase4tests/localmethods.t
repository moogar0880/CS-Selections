class A
{
  int i;
  int f(){ return 1; }
  int g()
  {
  	out f();
  }
}

int main()
{
  A a;

  a = new A();

  a.g();

  out 42;
}
