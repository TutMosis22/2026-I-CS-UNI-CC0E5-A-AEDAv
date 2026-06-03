#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// ======================================================
// Nodo para lista doblemente enlazada
// ======================================================

template <typename T>
class DLLNode : public LLNode<T>{
public:
    using Node = DLLNode<T>;

private:
    Node* m_pPrev;

public:
    DLLNode()
        : LLNode<T>(), m_pPrev(nullptr) {}

    DLLNode(T data, Ref ref,
            Node* next = nullptr,
            Node* prev = nullptr)
        : LLNode<T>(data, ref, next),
          m_pPrev(prev) {}

    Node* getPrev() const {
        return m_pPrev;
    }

    void setPrev(Node* prev) {
        m_pPrev = prev;
    }

    Node*& getPrevRef() {
        return m_pPrev;
    }
};

// ======================================================
// Traits
// ======================================================

template <typename T>
struct AscendingDLLTrait :
    public BaseTrait<DLLNode<T>, less<T>> {
};

template <typename T>
struct DescendingDLLTrait :
    public BaseTrait<DLLNode<T>, greater<T>> {
};

// ======================================================
// DoubleLinkedList
// ======================================================

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:

    // --------------------------------------------------
    // TAREA: Heredar constructores
    // --------------------------------------------------
    using LinkedList<Trait>::LinkedList;

    // --------------------------------------------------
    // TAREA: Copy Constructor
    // --------------------------------------------------
    DoubleLinkedList(const DoubleLinkedList&) = default;

    // --------------------------------------------------
    // TAREA: Move Constructor
    // --------------------------------------------------
    DoubleLinkedList(DoubleLinkedList&&) = default;

    // --------------------------------------------------
    // TAREA: Copy Assignment
    // --------------------------------------------------
    DoubleLinkedList& operator=(const DoubleLinkedList&) = default;

    // --------------------------------------------------
    // TAREA: Move Assignment
    // --------------------------------------------------
    DoubleLinkedList& operator=(DoubleLinkedList&&) = default;

    // --------------------------------------------------
    // Destructor
    // --------------------------------------------------
    ~DoubleLinkedList() = default;
};

#endif // __DOUBLELINKEDLIST_H__