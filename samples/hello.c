// Computes the cost of a simple order, including tax, and prints a
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

    printf("Subtotal: %d\n", subtotal);
    printf("Total with tax: %d\n", total);

    return 0;
}
