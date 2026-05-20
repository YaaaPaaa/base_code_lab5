#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
};

/*--------------------  AccessFunctionType  -------------------------*/

typedef BinarySearchTree* (*AccessFunction)(const BinarySearchTree*);

/*--------------------  ChildAccessorsType  -------------------------*/

typedef struct s_ChildAccessors {
    AccessFunction child;
    AccessFunction opposite;
} ChildAccessors;

/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    t->key = key;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
    *t=NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/*Exercice 1*/
/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
	BinarySearchTree* parent = NULL;
    BinarySearchTree** cur = t;
    while(!bstree_empty(*cur) && bstree_key(*cur) != v){
        parent = *cur;
        if(bstree_key(*cur) > v) cur = &((*cur)->left);
        else cur = &((*cur)->right);
    }
    *cur = bstree_cons(NULL,NULL,v);
    (*cur)->parent = parent;
}

/*Exercice 4 : search, successor et predecessor*/
const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    while(!bstree_empty(t)){
        if(v < bstree_key(t)) t = bstree_left(t);
        else if(v > bstree_key(t)) t = bstree_right(t);
        else return t;
    }
    return bstree_create();
}

BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessors access) {
    if (!bstree_empty(x)){
        if(!bstree_empty(access.child(x))){
            x = access.child(x);
            while (!bstree_empty(access.opposite(x))) x = access.opposite(x);
            return (BinarySearchTree*)x;
        }

        const BinarySearchTree* p = bstree_parent(x);

        while(!bstree_empty(p) && x == access.child(p)){
            x = p;
            p = bstree_parent(p);
        }

        return (BinarySearchTree*)p;   
    }
    return bstree_create();
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    ChildAccessors access = {
        .child = bstree_right,
        .opposite = bstree_left
    };
    return find_next(x,access);
}


const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    ChildAccessors access = {
        .child = bstree_left,
        .opposite = bstree_right
    };
    return find_next(x,access);
}

/*Exercice 5 : swap_nodes, remove_node, remove*/
void bstree_swap_nodes(ptrBinarySearchTree* t, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    BinarySearchTree *fromParent = from->parent;
    BinarySearchTree *fromLeft   = from->left;
    BinarySearchTree *fromRight  = from->right;

    BinarySearchTree *toParent = to->parent;
    BinarySearchTree *toLeft   = to->left;
    BinarySearchTree *toRight  = to->right;

    int fromIsLeft = (!bstree_empty(fromParent) && bstree_left(fromParent) == from);
    int toIsLeft = (!bstree_empty(toParent) && bstree_left(toParent) == to);

    //Cas particulier : from parent de to
    if(toParent == from) {
        to->parent = fromParent;

        if(!bstree_empty(fromParent)){
            if (fromIsLeft) fromParent->left = to;
            else fromParent->right = to;
        } else *t = to;

        from->parent = to;

        if(from->left == to) {
            to->left = from;
            to->right = fromRight;

            if (fromRight) fromRight->parent = to;

            from->left = toLeft;
            from->right = toRight;
        } else {
            to->right = from;
            to->left = fromLeft;

            if (fromLeft) fromLeft->parent = to;

            from->left = toLeft;
            from->right = toRight;
        }
        
        if (toLeft) toLeft->parent = from;
        if (toRight)toRight->parent = from;
    } else if (fromParent == to) bstree_swap_nodes(t, to, from); //Cas symétrique : to parent de from

    //Cas général
    else {
        from->parent = toParent;
        to->parent = fromParent;

        from->left = toLeft;
        from->right = toRight;

        to->left = fromLeft;
        to->right = fromRight;

        //Mise à jour des parents
        if(from->left) from->left->parent = from;
        if(from->right) from->right->parent = from;
        if(to->left) to->left->parent = to;
        if(to->right) to->right->parent = to;

        //Reconnexion depuis les parents
        if(!bstree_empty(fromParent)){
            if(fromIsLeft) fromParent->left = to;
            else fromParent->right = to;
        } else *t = to;

        if(!bstree_empty(toParent)) {
            if (toIsLeft) toParent->left = from;
            else toParent->right = from;
        } else *t = from;
    }    
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    //Si il y a deux fils, on intervertit avec le successeur pour être dans le cas 1 ou 2
    if(current->left != NULL && current->right != NULL){
        ptrBinarySearchTree successeur = (ptrBinarySearchTree)bstree_successor(current);
        bstree_swap_nodes(t, current, successeur);
    } 

    //Cas 1 ou 2 : Suppression d'un noeud avec au plus un fils
    ptrBinarySearchTree child;
    if(!bstree_empty(bstree_left(current))) child = bstree_left(current);
    else child = bstree_right(current);

    if(!bstree_empty(child)) child->parent = current->parent;

    if(bstree_empty(bstree_parent(current)))*t = child;
    else if(current == bstree_left(bstree_parent((current)))) current->parent->left = child;
    else current->parent->right = child;

    free(current);
}

// Recherche puis suppression de la valeur v
void bstree_remove(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree rmNode = (ptrBinarySearchTree)bstree_search(*t, v);
    if(!bstree_empty(rmNode)) bstree_remove_node(t, rmNode);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

/*Exercice 2 : Parcours en profondeur (prefix, infix et postfix)*/
void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)){
        f(t, environment);
        bstree_depth_prefix(bstree_left(t),f,environment);
        bstree_depth_prefix(bstree_right(t),f,environment);
    }
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)){
        bstree_depth_infix(bstree_left(t),f,environment);
        f(t, environment);
        bstree_depth_infix(bstree_right(t),f,environment);
    }
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)){
        bstree_depth_postfix(bstree_left(t),f,environment);
        bstree_depth_postfix(bstree_right(t),f,environment);
        f(t, environment);
    }
}

/*Exercice 3*/
void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    assert(!bstree_empty(t));

    Queue* q = create_queue();
    queue_push(q, (void*)t);
    
    while(!queue_empty(q)){
        const BinarySearchTree* noeud = queue_top(q);
        queue_pop(q);
        f(noeud, environment);
        if(noeud->left != NULL) queue_push(q, noeud->left);
        if(noeud->right != NULL)queue_push(q, noeud->right);
    }

    delete_queue(&q);
}

/*Exercice 5*/
void bstree_iterative_depth_infix(const BinarySearchTree *t, OperateFunctor f,void *userData){
    const BinarySearchTree *current = t;
    const BinarySearchTree *previous = bstree_create();
    const BinarySearchTree *next = bstree_create();

    while (!bstree_empty(current)){
        //On descend depuis le parent
        if(previous == bstree_parent(current)){
            if(!bstree_empty(bstree_left(current))) next = bstree_left(current);
            else{
                //Pas de fils gauche :
                f(current, userData);

                if(!bstree_empty(bstree_right(current))) next = bstree_right(current);
                else next = bstree_parent(current);
            }
        }

        //On remonte du fils gauche
        else if(previous == bstree_left(current)) {
            //Sous-arbre gauche terminé
            f(current, userData);

            if(!bstree_empty(bstree_right(current))) next = bstree_right(current);
            else next = bstree_parent(current);
        }

        //On remonte du fils droit
        else next = bstree_parent(current);

        previous = current;
        current = next;
    }
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	(void)collection; (void)direction;
	return NULL;
}

/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}