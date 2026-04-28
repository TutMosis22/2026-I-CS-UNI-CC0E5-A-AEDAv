#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// Circular LinkedList
template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
    using Base = LinkedList<Trait>;
    using Node = typename Base::Node;
    using value_type = typename Base::value_type;

public:

    // insert adaptado a circular
    void insert(const value_type& value, Ref ref) {
        unique_lock<shared_mutex> lock(this->m_mtx);

        if (!this->m_pRoot) {
            this->m_pRoot = new Node(value, ref);
            this->m_tail = this->m_pRoot;
            this->m_tail->setNext(this->m_pRoot);
            this->m_size = 1;
            return;
        }

        Node* newNode = new Node(value, ref, this->m_pRoot);
        this->m_tail->setNext(newNode);
        this->m_pRoot = newNode;
        this->m_size++;
    }

    // recorrido circular (n vueltas)
    template <typename Func>
    void circularForEach(Func func, int vueltas = 1) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) return;

        Node* current = this->m_pRoot;
        int count = 0;

        do {
            func(current->getDataRef());
            current = current->getNext();

            if (current == this->m_pRoot) count++;

        } while (count < vueltas);
    }
};

#endif