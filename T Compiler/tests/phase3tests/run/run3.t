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
  out a1.i;
  a1.i = 21;
  out 99;
  out a1.i;
  a2 = a1;
  out 76;
  out a2.i;

  if (a1 == a2)
  {
    out 17;
    i = 21;
  }
  else
  {
    i = 1;
  }
  out 103;
  out i;
  out a2.i;
  i = i + a2.i;
  out 274;
  out i;

  return 0;
}

