void exported_from_main(const char *);

void
module_func(const char *msg)
{
        exported_from_main(msg);
}
