
void
foo(int *p)
{
	*p = 1;  /* NULL dereference */
}

int
main(int argc, char *argv[])
{
	foo(0);
}
