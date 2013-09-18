// int main()
// {
// 	out 42;
//     return 0;
// }

class A
{
  int i;
  A(){ out 42; }
}

int main()
{
  A a;

  a = new A();

  a.i = 42;

  out a.i;

  return 0;
}
