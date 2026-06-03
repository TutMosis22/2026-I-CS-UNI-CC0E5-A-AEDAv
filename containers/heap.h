#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <utility>
#include <tuple>
#include "util.h"
#include "../types.h"
#include "traits.h"
#include "vector.h"
#include <shared_mutex>
using namespace std;

template <typename T>
struct MinHeapTrait : public BaseHeapTrait<T, less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseHeapTrait<T, greater<T>> {};

template<typename T>
class HeapNode{
public:
    using value_type = T;

private:
    value_type m_data;
    Ref m_ref;

public:
    HeapNode()
        : m_data(value_type()), m_ref(0) {}

    HeapNode(value_type data, Ref ref)
        : m_data(data), m_ref(ref) {}

    value_type getData() const {
        return m_data;
    }

    value_type& getDataRef() {
        return m_data;
    }

    Ref getRef() const {
        return m_ref;
    }

    void setData(value_type data) {
        m_data = data;
    }

    void setRef(Ref ref) {
        m_ref = ref;
    }
};

// =====================================================
// operator<< para HeapNode
// Necesario para que Vector<HeapNode<T>> pueda imprimirse
// =====================================================

template<typename T>
ostream& operator<<(ostream& os, const HeapNode<T>& node){
    return os << node.getData();
}

template<typename Trait>
class Heap{
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;
    using Node       = HeapNode<value_type>;
    
private:
    Vector<Node> m_vec;
    Comp          m_comp;
    mutable shared_mutex m_mtx;
public:
    Heap() : m_vec(), m_comp() {}
    ~Heap() {}

    void heapifyUp(size_t index);
    void heapifyDown(size_t index);

    void insert(value_type value, Ref ref);
    
    // Extrae el elemento de mayor o menor prioridad (depende del heap)
    void extract(); 
    
    // Obtiene el elemento de mayor o menor prioridad (depende del heap) 
    // sin removerlo
    Node peek();
    
    bool isEmpty();
    size_t size();
    string toString();
};

// =====================================================
// Heap::isEmpty
// =====================================================

template<typename Trait>
bool Heap<Trait>::isEmpty(){
    shared_lock<shared_mutex> lock(m_mtx);
    return m_vec.size() == 0;
}

// =====================================================
// Heap::size
// =====================================================

template<typename Trait>
size_t Heap<Trait>::size(){
    shared_lock<shared_mutex> lock(m_mtx);
    return m_vec.size();
}

// =====================================================
// Heap::peek
// =====================================================

template<typename Trait>
typename Heap<Trait>::Node Heap<Trait>::peek(){
    shared_lock<shared_mutex> lock(m_mtx);

    if(m_vec.size() == 0)
        throw runtime_error("Heap vacio");

    return m_vec[0].getData();
}

// =====================================================
// Heap::toString
// =====================================================

template<typename Trait>
string Heap<Trait>::toString(){
    shared_lock<shared_mutex> lock(m_mtx);

    ostringstream oss;

    oss << "[";

    for(size_t i = 0; i < m_vec.size(); i++){

        if(i > 0)
            oss << ",";

        oss << m_vec[i].getData().getData();    //VectorNode -> HeapNode-> int
    }

    oss << "]";

    return oss.str();
}

// heapifyUp

template<typename Trait>
void Heap<Trait>::heapifyUp(size_t index){

    while(index > 0){

        size_t parent = (index - 1) / 2;

        if(m_comp(
            m_vec[index].getData().getData(),   //VectorNode ->HeapNode ->int
            m_vec[parent].getData().getData()
        )){
            swap(m_vec[index], m_vec[parent]);
            index = parent;
        }
        else{
            break;
        }
    }
}

//insert

template<typename Trait>
void Heap<Trait>::insert(value_type value, Ref ref){

    unique_lock<shared_mutex> lock(m_mtx);

    m_vec.push_back(Node(value, ref), ref);

    heapifyUp(m_vec.size() - 1);
}

//heapifyDown

template<typename Trait>
void Heap<Trait>::heapifyDown(size_t index){

    size_t n = m_vec.size();

    while(true){

        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;

        size_t best = index;

        if(left < n &&
           m_comp(
               m_vec[left].getData().getData(),
               m_vec[best].getData().getData()
           )){
            best = left;
        }

        if(right < n &&
           m_comp(
               m_vec[right].getData().getData(),
               m_vec[best].getData().getData()
           )){
            best = right;
        }

        if(best == index)
            break;

        swap(m_vec[index], m_vec[best]);

        index = best;
    }
}

//extract

template<typename Trait>
void Heap<Trait>::extract(){

    unique_lock<shared_mutex> lock(m_mtx);

    if(m_vec.size() == 0)
        throw runtime_error("Heap vacio");

    if(m_vec.size() == 1){
        m_vec.pop_back();
        return;
    }

    m_vec[0] = m_vec[m_vec.size() - 1];

    m_vec.pop_back();

    heapifyDown(0);
}

// =====================================================
// operator
// =====================================================

template<typename Trait>
ostream& operator<<(ostream& os, Heap<Trait>& heap){

    os << heap.toString();

    return os;
}

// =====================================================
// operator>>
// Formato:
// 10 20 30 40
// =====================================================

template<typename Trait>
istream& operator>>(istream& is, Heap<Trait>& heap){

    typename Heap<Trait>::value_type value;

    while(is >> value){
        heap.insert(value, 0);
    }

    return is;
}

#endif // __HEAP_H__