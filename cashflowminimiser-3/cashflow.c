#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PEOPLE 100
#define MAX_NAME_LEN 50
#define EPSILON 0.001 // Tolerance for float comparison

// Structure to hold a person's name and their net balance
typedef struct {
    char name[MAX_NAME_LEN];
    double net_balance;
} PersonBalance;

// Function to find a person's index in an array, or add them if new
int get_person_index(const char* name, char people_names[][MAX_NAME_LEN], int* num_people) {
    for (int i = 0; i < *num_people; ++i) {
        if (strcmp(people_names[i], name) == 0) {
            return i;
        }
    }
    // If not found, add the new person
    if (*num_people < MAX_PEOPLE) {
        strncpy(people_names[*num_people], name, MAX_NAME_LEN - 1);
        people_names[*num_people][MAX_NAME_LEN - 1] = '\0';
        return (*num_people)++;
    }
    return -1; // Error case: too many people
}

// Recursive function to settle debts
void min_cashflow_rec(PersonBalance balances[], int num_people) {
    int max_credit_idx = -1, max_debit_idx = -1;
    double max_credit = 0.0, max_debit = 0.0;

    // Find the person who is owed the most and the person who owes the most
    for(int i = 0; i < num_people; i++) {
        if(balances[i].net_balance > max_credit) {
            max_credit = balances[i].net_balance;
            max_credit_idx = i;
        }
        if(balances[i].net_balance < max_debit) {
            max_debit = balances[i].net_balance;
            max_debit_idx = i;
        }
    }

    // If all amounts are settled (within a small tolerance for floating point errors)
    if (fabs(max_credit) < EPSILON && fabs(max_debit) < EPSILON) {
        return;
    }

    double settlement_amount = fmin(-max_debit, max_credit);

    // Update balances
    balances[max_debit_idx].net_balance += settlement_amount;
    balances[max_credit_idx].net_balance -= settlement_amount;

    // Print the transaction
    printf("💰 %-10s pays $%-7.2f to %-10s\n", balances[max_debit_idx].name, settlement_amount, balances[max_credit_idx].name);

    // Recur for the remaining amounts
    min_cashflow_rec(balances, num_people);
}

int main() {
    int num_transactions;
    if (scanf("%d", &num_transactions) != 1 || num_transactions < 0) {
        fprintf(stderr, "Error: Invalid number of transactions.\n");
        return 1;
    }

    char people_names[MAX_PEOPLE][MAX_NAME_LEN];
    double net_amounts[MAX_PEOPLE] = {0.0};
    int num_people = 0;

    for (int i = 0; i < num_transactions; ++i) {
        char payer_name[MAX_NAME_LEN], receiver_name[MAX_NAME_LEN];
        double amount;
        if (scanf("%s %s %lf", payer_name, receiver_name, &amount) != 3) {
            fprintf(stderr, "Error: Invalid transaction format on line %d.\n", i + 1);
            return 1;
        }
        if (amount <= 0) {
            fprintf(stderr, "Error: Transaction amount must be positive on line %d.\n", i + 1);
            return 1;
        }

        int payer_idx = get_person_index(payer_name, people_names, &num_people);
        int receiver_idx = get_person_index(receiver_name, people_names, &num_people);

        if (payer_idx == -1 || receiver_idx == -1) {
            fprintf(stderr, "Error: Maximum number of people (%d) reached.\n", MAX_PEOPLE);
            return 1;
        }
        
        net_amounts[payer_idx] -= amount;
        net_amounts[receiver_idx] += amount;
    }

    // Print Net Balances
    printf("📊 Net Balances:\n------------------------------------------------\n");
    PersonBalance balances[MAX_PEOPLE];
    for(int i = 0; i < num_people; i++) {
        strncpy(balances[i].name, people_names[i], MAX_NAME_LEN);
        balances[i].net_balance = net_amounts[i];
        if (net_amounts[i] > EPSILON) {
            printf("   %-10s will RECEIVE $%.2f\n", people_names[i], net_amounts[i]);
        } else if (net_amounts[i] < -EPSILON) {
            printf("   %-10s will PAY     $%.2f\n", people_names[i], -net_amounts[i]);
        } else {
            printf("   %-10s is SETTLED\n", people_names[i]);
        }
    }
    printf("------------------------------------------------\n\n");

    // Print Settlements
    printf("⚖️ Settlements (Minimum Transactions):\n");
    min_cashflow_rec(balances, num_people);

    return 0;
}

