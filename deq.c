#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {
    Head, Tail, Ends
} End;

typedef struct Node {
    struct Node *np[Ends];        // next/prev neighbors
    Data data;
} *Node;

typedef struct {
    Node ht[Ends];            // head/tail nodes
    int len;
} *Rep;

static Rep rep(Deq q) {
    if (!q) ERROR("zero pointer");
    return (Rep) q;
}

/**
 * Append a data element onto the beginning or end of list, or REPresentation of the list
 * @param r - Pointer to the entire list, or REPresentation of the list
 * @param e - Start counting from beginning or end of list, or REPresentation of the list
 * @param d - Data to be inserted into list or REPresentation of the list
 */
static void put(Rep r, End e, Data d) {
    // clone node being appended to the list, or REPresentation of the list
    Node t = r->ht[e];
    // instantiate a new node with data d to be added to the list, or REPresentation of the list
    Node n = (Node) malloc(sizeof(Node));
    // store data in Node n to be store in the list, or REPresentation of the list
    n->data = d;
    if (r->len != 0) {
        // add Node t as next or previous respectively in Node n in the list, or REPresentation of the list
        n->np[(e == Head) ? Tail : Head] = t;
        // set Node t's next or previous respectively to Node n in the list, or REPresentation of the list
        t->np[(e == Head) ? Head : Tail] = n;
    }
    // add Node n back into the list, or REPresentation of the list
    if (r->len == 0) {
        r->ht[Head] = n;
        r->ht[Tail] = n;
    } else {
        r->ht[e] = n;
    }
    // increment length of the list, or REPresentation of the list
    r->len++;
}

/**
 * Get the element at index i starting from the beginning or end of the list, or REPresentation of the list
 * @param r - Pointer to the entire list, or REPresentation of the list
 * @param e - Start counting from beginning or end of list, or REPresentation of the list
 * @param i - Index to count to from beginning or end of list, or REPresentation of the list
 * @return - Data found at index i of the list, or REPresentation of the list
 */
static Data ith(Rep r, End e, int i) {
    // initialized Node n to the head or tail of the list, or REPresentation of the list
    Node n = (e == Head) ? r->ht[Head] : r->ht[Tail];
    // until the end of the list is reached, iterate through the list, or REPresentation of the list
    for (int j = 0; j < r->len; j++) {
        // if the index is found, return the data from said index of the list, or REPresentation of the list
        if (j == i)
            return n->data;
        // set Node n to the next Node in the list, or REPresentation of the list
        n = (e == Head) ? n->np[Tail] : n->np[Head];
    }
    // return 0 if i is longer than the length of the list, or REPresentation of the list
    return 0;
}

/**
 * Get and remove the element at the beginning or end of the list, or REPresentation of the list
 * @param r - Pointer to the entire list, or REPresentation of the list
 * @param e - Specify whether to retrieve the element at the beginning or end of the list, or REPresentation of the list
 * @return - Data retrieved from the list, or REPresentation of the list
 */
static Data get(Rep r, End e) {
    // get data from desired Node in the list, or REPresentation of the list
    Data d = r->ht[e]->data;
    if (r->len != 1) {
        // assign next node to the desired end of the list, or REPresentation of the list
        r->ht[e] = r->ht[e]->np[(e == Head) ? Tail : Head];
        // remove the previous end Node from the list, or REPresentation of the list
        r->ht[e]->np[(e == Head) ? Head : Tail] = 0;
    } else {
        r->ht[Head] = 0;
        r->ht[Tail] = 0;
    }
    // decrement the length of the list, or REPresentation of the list
    r->len--;
    return d;
}

/**
 * Remove the specified data element from the list, or REPresentation of the list
 * @param r - Pointer to the entire list, or REPresentation of the list
 * @param e - Start counting from beginning or end of list, or REPresentation of the list
 * @param d - Data to be removed from the list, or REPresentation of the list
 * @return - Data removed from the list, or REPresentation of the list
 */
static Data rem(Rep r, End e, Data d) {
    // assign Node n to desired end of the list, or REPresentation of the list
    Node n = r->ht[e];
    // iterate through the list, or REPresentation of the list
    for (int i = 0; i < r->len; i++) {
        // if the desired data is found in the list, or REPresentation of the list
        if (n->data == d) {
            // if the node to be removed is at the head or tail of the list, or REPresentation of the list
            if (n->np[Head] == 0 || n->np[Tail] == 0) {
                // use get method to remove the node from the list, or REPresentation of the list
                return get(r, (n->np[Head] == 0) ? Head : Tail);
            } else {
                // otherwise, reassign the pointers from the surrounding Nodes in the list, or REPresentation of the list
                n->np[Head]->np[Tail] = n->np[Tail];
                n->np[Tail]->np[Head] = n->np[Head];
            }
            // decrement the length counter of the list, or REPresentation of the list
            r->len--;
            return n->data;
        }
        // set Node n to the next Node in the list, or REPresentation of the list
        n = (e == Head) ? n->np[Tail] : n->np[Head];
    }
    return 0;
}

extern Deq deq_new() {
    Rep r = (Rep) malloc(sizeof(*r));
    if (!r) ERROR("malloc() failed");
    r->ht[Head] = 0;
    r->ht[Tail] = 0;
    r->len = 0;
    return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) { put(rep(q), Head, d); }

extern Data deq_head_get(Deq q) { return get(rep(q), Head); }

extern Data deq_head_ith(Deq q, int i) { return ith(rep(q), Head, i); }

extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q), Head, d); }

extern void deq_tail_put(Deq q, Data d) { put(rep(q), Tail, d); }

extern Data deq_tail_get(Deq q) { return get(rep(q), Tail); }

extern Data deq_tail_ith(Deq q, int i) { return ith(rep(q), Tail, i); }

extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q), Tail, d); }

extern void deq_map(Deq q, DeqMapF f) {
    for (Node n = rep(q)->ht[Head]; n; n = n->np[Tail])
        f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
    if (f) deq_map(q, f);
    Node curr = rep(q)->ht[Head];
    while (curr) {
        Node next = curr->np[Tail];
        free(curr);
        curr = next;
    }
    free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
    char *s = strdup("");
    for (Node n = rep(q)->ht[Head]; n; n = n->np[Tail]) {
        char *d = f ? f(n->data) : n->data;
        char *t;
        asprintf(&t, "%s%s%s", s, (*s ? " " : ""), d);
        free(s);
        s = t;
        if (f) free(d);
    }
    return s;
}
