int func_in_bar(int n)
{
	return n + 5;
}

const char *func_in_main();

const char *
call_func_in_main()
{
	return func_in_main();
}
