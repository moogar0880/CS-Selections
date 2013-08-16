int main()
{
  i i;

  i = new i();

  i.i();

  i.i = i.i - 22;

  delete i;
}

class i
{
  int i;
  int i() { i = i + 42; }
  i() { i = 22; }
  ~i() { out i; }
}

