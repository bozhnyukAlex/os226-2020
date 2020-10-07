#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "mylist.h"

struct List createList() {
    struct List list;
    list.head = NULL;
    list.end = NULL;
    list.length = 0;
    return list;
}

struct Node* createNode(struct task* value) {
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("ERROR");
        exit(0);
    }
    node->data = value;
    node->next = NULL;
    return node;
}
struct Node* getN(struct List* list, size_t n) {
    struct Node* curr = list->head;
    size_t index = 0;
    while (curr) {
        if (index == n) {
            return curr;
        }
        curr = curr->next;
        index++;
    }
    return NULL;
}
void insertToBegin(struct List* list, struct Node* node) {
    node->next = list->head;
    list->head = node;
    list->length++;
    if (list->length == 1) {
        list->end = node; ///if not, list->end already exists
    }
}
void insertAfterEl(struct List* list, size_t afterNum, struct task* newValue) {
    struct Node* after = getN(list, afterNum);
    struct Node* newNode = createNode(newValue);
    newNode->next = after->next;
    after->next = newNode;
    list->length++;
    if (list->end == after) {
        list->end = newNode;
    }
}

void push(struct List* list, struct task* newValue) {
    if (list->length == 0) {
        list->head = createNode(newValue);
        list->head->next = NULL;
        list->end = list->head;
        list->length++;
        return;
    }
    struct Node *curr = list->head;
    struct Node* newNode = createNode(newValue);
    list->end->next = newNode;
    list->end = newNode;
    list->length++;
}

void deleteNode(struct List* list, size_t numDelete) {
    struct Node* toDelete = getN(list, numDelete);
    struct Node* curr = list->head;
    if (curr == toDelete) {
        list->head = curr->next;
        free(toDelete);
        list->length--;
        return;
    }
    while (curr) {
        if (curr->next == toDelete) {
            curr->next = curr->next->next;
            free(toDelete);
            list->length--;
            return;
        }
        curr = curr->next;
    }
}

void deleteNodeByValue(struct List* list, struct task* value) {
    struct Node* curr = list->head;
    if (list->length == 1) {
        list->head = curr->next;
        list->end = list->head;
        list->length = 0;
        free(curr);
        return;
    }
    while (curr) {
        if(curr->next->data == value) {
            free(curr->next);
            curr->next = curr->next->next;
            list->length--;
            if (list->length == 1) {
                list->end = list->head;
            }
            return;
        }
        curr = curr->next;
    }
}

void deleteHead(struct List* list) {
    if (list->length == 0) {
        return;
    }
    struct Node* toDelete = list->head;
    list->head = toDelete->next;
    list->length--;
    if (list->length == 1) {
        list->end = list->head;
    }
    free(toDelete);
}

void createCycle(struct List* list, size_t to) {
    list->end->next = getN(list, to);
}

int checkCycle(struct List* list) {
    struct Node* left = list->head;
    struct Node* right = list->head;
    if (left == NULL) {
        return 0;
    }
    while (1) {
        right = right->next->next;
        left = left->next;
        if (right->next == NULL || right->next->next == NULL || left->next == NULL) {
            return 0;
        }
        if (right == left) {
            return 1;
        }

    }
}

void deleteCycle(struct List* list) {
    list->end->next = NULL;
}

void clearList(struct List* list) {
    if (list->length == 0) {
        return;
    }
    struct Node* curr = list->head;
    while (curr) {
        struct Node *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    list->head = NULL;
    list->end = NULL;
    list->length = 0;
}

int indexOf(struct List *list, struct Node *toFind) {
    int i = 0;
    struct Node *curr = list->head;
    while (i < list->length) {
        if (curr == toFind) {
            return i;
        }
        curr = curr->next;
        i++;
    }
    return NULL;
}
void shiftRightPiece(struct List *list, int pos1, int pos2, int shift) {
    struct Node* inPos1 = getN(list, pos1);
    struct Node* inPos2 = getN(list, pos2);
    if (pos1 == 0) {
        struct Node* afterPos2 = getN(list, pos2 + 1);
        struct Node* toShift = getN(list, pos2 + shift + 1);
        struct Node* beforeToShift = getN(list, pos2 + shift);
        list->head = afterPos2;
        beforeToShift->next = inPos1;
        inPos2->next = toShift;
        return;
    }
    struct Node* afterPos2 = getN(list, pos2 + 1);
    struct Node* beforePos1 = getN(list, pos1 - 1);
    struct Node* toShift = getN(list, pos2 + shift + 1);
    struct Node* beforeToShift = getN(list, pos2 + shift);

    beforePos1->next = afterPos2;
    beforeToShift->next = inPos1;
    inPos2->next = toShift;
    


}

void printList(struct List *list) {
    if (list->length == 0) {
        printf("List is empty\n");
        return;
    }
    struct Node* curr = list->head;
    int index = 0;
    while (curr) {
        printf("i: %d, index: %d, prior: %d, deadline: %d, cnt: %d\n", index,  curr->data->index, curr->data->priority, curr->data->deadline, *((int*)curr->data->ctx));
        index++;
        curr = curr->next;
    }
}



void reverseList(struct List* list) {
    if (list->length == 0 || list->length == 1) {
        return;
    }
    struct Node* nextNode = NULL;
    struct Node* curr = list->head;
    struct Node* prev = NULL;
    list->end = curr;
    while (curr) {
        nextNode = curr->next;
        curr->next = prev;
        prev = curr;
        curr = nextNode;
    }

    list->head = prev;
}
