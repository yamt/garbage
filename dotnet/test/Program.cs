using System;
using System.Runtime.InteropServices;

class Dll
{
    [DllImport("libc.so.6")]
    public static extern int getpid();
}

namespace src
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("pid is " + Dll.getpid());
        }
    }
}
