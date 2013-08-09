class A
{
  int i;
}

int main()
{
  A a1;
  A a2;
  int i;

  a1 = new A();

  a1.i = 21;

  a2 = a1;

  if (null == a2)
  {
    i = 1;
  }
  else
  {
    i = 21;
  }

  i = i + a2.i;

  out i;

  return 0;
}

