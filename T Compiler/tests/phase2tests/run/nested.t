int main()
{
  int i;
  int j;

  i = 42;
  j = 1;

  if (i == 42)
  {
    while (j < 42) j = j + 1;
    out j;
  }
  else out i+1;

  j = 4;
  while (j > 0)
  {
    if (i == 43) out i+1;
    else out i;
    j = j - 1;
  }
}

