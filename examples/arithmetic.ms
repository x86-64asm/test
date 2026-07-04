// Basic Arithmetic Operations
use stdlib;

fn add(a: int, b: int): int {
    return a + b;
}

fn subtract(a: int, b: int): int {
    return a - b;
}

fn multiply(a: int, b: int): int {
    return a * b;
}

fn divide(a: int, b: int): int {
    return a / b;
}

fn main() {
    var x: int = 20;
    var y: int = 5;
    
    io.out("Add: ");
    io.out(add(x, y));
    
    io.out("Subtract: ");
    io.out(subtract(x, y));
    
    io.out("Multiply: ");
    io.out(multiply(x, y));
    
    io.out("Divide: ");
    io.out(divide(x, y));
}
