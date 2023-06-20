void fill(int *);
int calc(const int *);

int
func()
{
        int buf[100000];
        fill(buf);
        return calc(buf);
}
