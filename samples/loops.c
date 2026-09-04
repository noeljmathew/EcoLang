// Demonstrates a loop accumulating a running total.
int main() {
    int total = 0;
    int i = 1;

    while (i <= 5) {
        total = total + i;
        printf("Added %d, running total = %d\n", i, total);
        i = i + 1;
    }

    printf("Final total: %d\n", total);
    return 0;
}
