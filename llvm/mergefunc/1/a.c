#define F(N)                                                                  \
        __attribute__((noinline)) int N(int x)                                \
        {                                                                     \
                switch (x) {                                                  \
                case 0:                                                       \
                case 2:                                                       \
                case 4:                                                       \
                case 6:                                                       \
                case 7:                                                       \
                        x = 1;                                                \
                        break;                                                \
                default:                                                      \
                        x = 0;                                                \
                        break;                                                \
                }                                                             \
                return x;                                                     \
        }

F(f)
F(g)
F(h)
