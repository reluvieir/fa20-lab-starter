#include <stddef.h>
#include "ll_cycle.h"

int ll_has_cycle(node *head) {
    /* your code here */
    if (!head) return 0;
    node *slow = head;
    node *fast = head;
    while(slow && fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return 1; // cycle detected
        }
    }
    return 0;
}