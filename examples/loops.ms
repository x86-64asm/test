// Loop Examples - Musab Language
use stdlib;

fn main() {
    // While loop - Print numbers 1 to 5
    io.out("While loop:");
    var i: int = 1;
    while (i <= 5) {
        io.out(i);
        i = i + 1;
    }
    
    // For loop - Sum 1 to 10
    io.out("Sum 1 to 10:");
    var sum: int = 0;
    for (var j: int = 1; j <= 10; j = j + 1) {
        sum = sum + j;
    }
    io.out(sum);
}
