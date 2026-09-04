// Demonstrates nested loops with a small multiplication table.
int main() {
    int outer = 1;
    while (outer <= 3) {
        int inner = 1;
        while (inner <= 3) {
            int product = outer * inner;
            printf("%d x %d = %d\n", outer, inner, product);
            inner = inner + 1;
        }
        outer = outer + 1;
    }

    return 0;
}
