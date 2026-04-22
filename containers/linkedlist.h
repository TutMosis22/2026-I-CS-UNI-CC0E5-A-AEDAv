#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

// ============================
// forward iterator (op++)
// ============================
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++(){
        this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// ============================
// Node
// ============================
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T   m_data;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_next(nullptr) {}
    LLNode(T data) : m_data(data), m_next(nullptr) {}
    LLNode(T data, Node *next) : m_data(data), m_next(next) {}

    T getData() const { return m_data; }
    T& getDataRef() { return m_data; }
    Node* getNext() const { return m_next; }
    Node*& getNextRef() { return m_next; }
    void setNext(Node* next) { m_next = next; }
};

// ============================
// Traits
// ============================
template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

// ============================
// LinkedList
// ============================
template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using Comp = typename Trait::Comp;

    using forward_iterator = LinkedListForwardIterator<LinkedList<Trait>>;

private:
    Node* m_pRoot = nullptr;
    Node* m_tail = nullptr;
    size_t m_size = 0;
    Comp m_comp;
    mutable shared_mutex m_mtx;

public:

    // ============================
    // constructor
    // ============================
    LinkedList() {}

    // ============================
    // copy constructor
    // ============================
    LinkedList(const LinkedList &other){
        shared_lock<shared_mutex> lock(other.m_mtx);

        Node* curr = other.m_pRoot;
        while(curr){
            push_back(curr->getData(), Ref());
            curr = curr->getNext();
        }
    }

    // ============================
    // move constructor
    // ============================
    LinkedList(LinkedList &&other){
        unique_lock<shared_mutex> lock(other.m_mtx);

        m_pRoot = other.m_pRoot;
        m_tail = other.m_tail;
        m_size = other.m_size;

        other.m_pRoot = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }

    // ============================
    // destructor seguro
    // ============================
    virtual ~LinkedList(){
        unique_lock<shared_mutex> lock(m_mtx);

        while(m_pRoot){
            Node* temp = m_pRoot;
            m_pRoot = m_pRoot->getNext();
            delete temp;
        }
    }

    // ============================
    // push_front
    // ============================
    void push_front(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);

        Node* node = new Node(value, m_pRoot);
        m_pRoot = node;

        if(m_size == 0)
            m_tail = node;

        m_size++;
    }

    // ============================
    // pop_front
    // ============================
    void pop_front(){
        unique_lock<shared_mutex> lock(m_mtx);

        if(!m_pRoot)
            throw out_of_range("Empty list");

        Node* temp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete temp;

        m_size--;

        if(m_size == 0)
            m_tail = nullptr;
    }

    // ============================
    // push_back
    // ============================
    void push_back(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);

        Node* node = new Node(value);

        if(!m_tail){
            m_pRoot = m_tail = node;
        }else{
            m_tail->setNext(node);
            m_tail = node;
        }

        m_size++;
    }

    // ============================
    // pop_back
    // ============================
    void pop_back(){
        unique_lock<shared_mutex> lock(m_mtx);

        if(!m_pRoot)
            throw out_of_range("Empty list");

        if(m_pRoot == m_tail){
            delete m_pRoot;
            m_pRoot = m_tail = nullptr;
        }else{
            Node* curr = m_pRoot;
            while(curr->getNext() != m_tail)
                curr = curr->getNext();

            delete m_tail;
            m_tail = curr;
            m_tail->setNext(nullptr);
        }

        m_size--;
    }

    // ============================
    // operator[]
    // ============================
    value_type& operator[](size_t index){
        shared_lock<shared_mutex> lock(m_mtx);

        if(index >= m_size)
            throw out_of_range("Index out of range");

        Node* curr = m_pRoot;
        for(size_t i = 0; i < index; ++i)
            curr = curr->getNext();

        return curr->getDataRef();
    }

    // ============================
    // size
    // ============================
    size_t size() const{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    // ============================
    // operator<<
    // ============================
    string toString() const{
        shared_lock<shared_mutex> lock(m_mtx);

        ostringstream oss;
        oss << "[";

        Node* curr = m_pRoot;
        bool first = true;

        while(curr){
            if(!first) oss << " -> ";
            oss << curr->getData();
            first = false;
            curr = curr->getNext();
        }

        oss << "]";
        return oss.str();
    }

    forward_iterator begin(){ return forward_iterator(this, m_pRoot); }
    forward_iterator end(){ return forward_iterator(this, nullptr); }

    // ============================
    // foreach
    // ============================
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }
};

// ============================
// operator<<
// ============================
template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

// ============================
// operator>> (persistencia)
// ============================
template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    char ch;
    typename Trait::value_type value;

    is >> ch; // [

    while(true){
        is >> value;
        list.push_back(value, Ref());

        is >> ch;
        if(ch == ']') break;

        is >> ch; // >
    }

    return is;
}

#endif