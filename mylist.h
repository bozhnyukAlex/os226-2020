#ifndef MYLIST_H_INCLUDED
#define MYLIST_H_INCLUDED
struct Node {
    void* data;
    struct Node* next;
};
struct List {
    struct Node* head;
    struct Node* end;
    size_t length;
};

struct List createList();
struct Node* createNode(void* value);
struct Node* getN(struct List* list, size_t n);
void insertToBegin(struct List* list, struct Node* node);
void insertAfterEl(struct List* list, size_t afterNum, void* newValue);
void deleteNode(struct List* list, size_t numDelete);
void clearList(struct List* list);
void deleteHead(struct List* list);
void push(struct List* list, void* newValue);
void createCycle(struct List* list, size_t to);
int checkCycle(struct List* list);
void deleteCycle(struct List* list);
void reverseList(struct List* list);
void printList(struct List *list);
int indexOf(struct List *list, struct Node *toFind);
void shiftRightPiece(struct List *list, int pos1, int pos2, int shift);
void deleteNodeByValue(struct List* list, void* value);

#endif // MYLIST_H_INCLUDED
