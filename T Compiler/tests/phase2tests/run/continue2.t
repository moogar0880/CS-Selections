int main()
{
  int i;
  int j;

  i = 5;
  j = 37;

  while (1)
  {
    out i + j;
    i = i - 1;
    j = 0;
    while (1)
    {
      j = j + 1;
      if (!(i + j == 42)) continue; else {}
      break;
    }
    if (!(i == 0)) continue; else {}
    break;
  }
}


