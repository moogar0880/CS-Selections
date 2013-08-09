int main()
{
  int i, j, k;
  int l;

  i = 1;
  j = 2;
  k = 3;

  l = i == j + k > j + i < k + !k;
  l = l + (i == j) + (k > j) + (i < k) + !k;
  l = l + !(i == j) + !(k > j) + !(i < k) + !!k;

  int n;

  n = l * 8 + 2;
  out n;
}
