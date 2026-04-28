#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// Nodo doble
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>> {
public:
    using Node = DLLNode<T>;

private:
    Node* m_prev;

public:
    DLLNode() : LLNode<T, Node>(), m_prev(nullptr) {}
    DLLNode(T data, Ref ref, Node* next = nullptr, Node* prev = nullptr)
        : LLNode<T, Node>(data, ref, next), m_prev(prev) {}

    Node* getPrev() const { return m_prev; }
    void setPrev(Node* prev) { m_prev = prev; }
};

// Traits
template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>> {
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>> {
    using Node = DLLNode<T>;
};

// Iterador forward
template <typename Container>
class DLLForwardIterator : public general_iterator<Container, DLLForwardIterator<Container>> {
public:
    using Parent = general_iterator<Container, DLLForwardIterator<Container>>;
    using Parent::Parent;

    DLLForwardIterator operator++() {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// Iterador backward
template <typename Container>
class DLLBackwardIterator : public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using Parent = general_iterator<Container, DLLBackwardIterator<Container>>;
    using Parent::Parent;

    DLLBackwardIterator operator++() {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// Double LinkedList
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
    using Base = LinkedList<Trait>;
    using Node = typename Trait::Node;
    using value_type = typename Trait::value_type;

public:

    // push_back adaptado doble
    void push_back(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(this->m_mtx);

        Node* newNode = new Node(value, ref);

        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
        } else {
            newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }

        this->m_size++;
    }

    // pop_back retorna valor
    tuple<value_type, Ref> pop_back() {
        unique_lock<shared_mutex> lock(this->m_mtx);

        if (!this->m_tail) throw runtime_error("Lista vacia");

        auto result = make_tuple(this->m_tail->getData(), this->m_tail->getRef());

        Node* temp = this->m_tail;

        this->m_tail = this->m_tail->getPrev();

        if (this->m_tail)
            this->m_tail->setNext(nullptr);
        else
            this->m_pRoot = nullptr;

        delete temp;
        this->m_size--;

        return result;
    }
};

#endif