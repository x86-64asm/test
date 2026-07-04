// Factorial - Musab Language
use stdlib;

fn factorial(n: int): int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() {
    var num: int = 5;
    var result: int = factorial(num);
    io.out("Factorial(");
    io.out(num);
    io.out(") = ");
    io.out(result);
}
