#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <cstddef>
#include <string>
#include "../types.h"
#include "traits.h"

using namespace std;

template<typename T>
struct BinaryTreeNode{
    using value_type = T;

    T m_data;
    Ref m_ref;
    BinaryTreeNode *m_pChild[2];

    BinaryTreeNode(T data, Ref ref = Ref())
        : m_data(data),
          m_ref(ref),
          m_pChild{nullptr, nullptr}
    {
    }
};

// Utilizar:
// AscendingTrait<BinaryTreeNode<T>>
// DescendingTrait<BinaryTreeNode<T>>

template<typename Trait>
class BinaryTree{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

private:
    Node* m_pRoot;
    Comp  m_comp;

protected:
    void internal_insert(Node*& pNode,
                         const value_type& data,
                         Ref ref);

    void internal_clear(Node* pNode);

    void internal_print(Node* pNode);

    // ===== TAREA: tamaño =====
    size_t internal_size(Node* pNode);

    // ===== TAREA: búsqueda =====
    bool internal_search(Node* pNode,
                         const value_type& value);

public:
    BinaryTree()
        : m_pRoot(nullptr)
    {
    }

    virtual ~BinaryTree(){
        internal_clear(m_pRoot);
    }

    void insert(const value_type& data, Ref ref){
        internal_insert(m_pRoot, data, ref);
    }

    void print(){
        internal_print(m_pRoot);
        cout << endl;
    }

    // ===== TAREA: size() =====
    size_t size(){
        return internal_size(m_pRoot);
    }

    // ===== TAREA: search() =====
    bool search(const value_type& value){
        return internal_search(m_pRoot, value);
    }
};

// ================================
// IMPLEMENTACIONES
// ================================

template<typename Trait>
void BinaryTree<Trait>::internal_insert(
    Node*& pNode,
    const value_type& data,
    Ref ref)
{
    if(pNode == nullptr){
        pNode = new Node(data, ref);
        return;
    }

    auto branch = !m_comp(data, pNode->m_data);

    internal_insert(
        pNode->m_pChild[branch],
        data,
        ref
    );
}

template<typename Trait>
void BinaryTree<Trait>::internal_clear(Node* pNode){
    if(pNode == nullptr)
        return;

    internal_clear(pNode->m_pChild[0]);
    internal_clear(pNode->m_pChild[1]);

    delete pNode;
}

template<typename Trait>
void BinaryTree<Trait>::internal_print(Node* pNode){
    if(pNode == nullptr)
        return;

    internal_print(pNode->m_pChild[0]);

    cout << "("
         << pNode->m_data
         << ","
         << pNode->m_ref
         << ") ";

    internal_print(pNode->m_pChild[1]);
}

// ================================
// TAREA: SIZE
// ================================

template<typename Trait>
size_t BinaryTree<Trait>::internal_size(Node* pNode){

    if(pNode == nullptr)
        return 0;

    return 1
         + internal_size(pNode->m_pChild[0])
         + internal_size(pNode->m_pChild[1]);
}

// ================================
// TAREA: SEARCH
// ================================

template<typename Trait>
bool BinaryTree<Trait>::internal_search(
    Node* pNode,
    const value_type& value)
{
    if(pNode == nullptr)
        return false;

    if(pNode->m_data == value)
        return true;

    auto branch = !m_comp(value, pNode->m_data);

    return internal_search(
        pNode->m_pChild[branch],
        value
    );
}

#endif // __BINARYTREE_H__