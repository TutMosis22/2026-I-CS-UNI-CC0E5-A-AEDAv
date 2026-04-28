#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

// Circular Double LinkedList
template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
    using Base = DoubleLinkedList<Trait>;
    using Node = typename Trait::Node;
    using value_type = typename Trait::value_type;

public:

    // insert circular doble
    void insert(const value_type& value, Ref ref) {
        unique_lock<shared_mutex> lock(this->m_mtx);

        Node* newNode = new Node(value, ref);

        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);

            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);

            this->m_tail = newNode;
        }

        this->m_size++;
    }

    // recorrido circular doble (forward/backward)
    template <typename Func>
    void circularForEach(Func func, int vueltas = 1, int dir = 1) {
        unique_lock<shared_mutex> lock(this->m_mtx);

        if (!this->m_pRoot) return;

        Node* current = this->m_pRoot;
        int count = 0;

        do {
            func(current->getDataRef());

            current = (dir == 1) ? current->getNext() : current->getPrev();

            if (current == this->m_pRoot) count++;

        } while (count < vueltas);
    }
};

#endif