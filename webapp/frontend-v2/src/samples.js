// Example programs for the "Load example" picker. hello.c is also the
// editor's default content.
export const SAMPLES = {
  "hello.c": `// Computes the cost of a simple order, including tax, and prints a
// summary. Demonstrates declarations, arithmetic, a function with
// parameters and a return value, and printf.
int addTax(int amount, int taxPercent) {
    return amount + (amount * taxPercent) / 100;
}

int main() {
    int itemPrice = 40;
    int quantity = 3;
    int subtotal = itemPrice * quantity;

    int total = addTax(subtotal, 5);

    printf("Subtotal: %d\\n", subtotal);
    printf("Total with tax: %d\\n", total);

    return 0;
}
`,
  "loops.c": `// Demonstrates a loop accumulating a running total.
int main() {
    int total = 0;
    int i = 1;

    while (i <= 5) {
        total = total + i;
        printf("Added %d, running total = %d\\n", i, total);
        i = i + 1;
    }

    printf("Final total: %d\\n", total);
    return 0;
}
`,
  "nested.c": `// Demonstrates nested loops with a small multiplication table.
int main() {
    int outer = 1;
    while (outer <= 3) {
        int inner = 1;
        while (inner <= 3) {
            int product = outer * inner;
            printf("%d x %d = %d\\n", outer, inner, product);
            inner = inner + 1;
        }
        outer = outer + 1;
    }

    return 0;
}
`,
  "recursion.c": `int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    printf("%d\\n", result);
    return 0;
}
`,
};

export const DEFAULT_FILE = "hello.c";
