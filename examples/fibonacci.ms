// Fibonacci Sequence - Musab Language
use stdlib;

fn fib(n: int): int {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    var n: int = 10;
    var result: int = fib(n);
    io.out("Fibonacci(");
    io.out(n);
    io.out(") = ");
    io.out(result);
}
