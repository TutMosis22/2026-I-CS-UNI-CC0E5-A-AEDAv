#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <cstddef>
#include <string>
#include "../types.h"
#include "traits.h"

using namespace std;

// =====================================================
// BinaryTreeNode: usa CRTP (Derived) para que m_pChild
// devuelva el tipo derivado directamente, sin static_cast
// disperso en BinaryTree/AVLTree.
//
// Derived = el tipo de nodo final (BinaryTreeNode<T> o
// AVLTreeNode<T>, que hereda de BinaryTreeNode<T,Derived>)
// =====================================================

template<typename T, typename Derived>
struct BinaryTreeNode{
    using value_type = T;

    T m_data;
    Ref m_ref;
    Derived *m_pChild[2];

    BinaryTreeNode(T data, Ref ref = Ref())
        : m_data(data),
          m_ref(ref),
          m_pChild{nullptr, nullptr}
    {
    }

    // child: acceso tipado, ya devuelve Derived* directamente
    Derived*& child(int dir){
        return m_pChild[dir];
    }
};

// Utilizar:
// AscendingTrait<BinaryTreeNode<T,T>>
// DescendingTrait<BinaryTreeNode<T,T>>

template<typename Trait>
class BinaryTree{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

protected:
    Node* m_pRoot;
    Comp  m_comp;

    // Inserción recursiva
    // virtual: permite que AVLTree sobreescriba con rebalanceo
    virtual void internal_insert(Node*& pNode,
                                 const value_type& data,
                                 Ref ref);

    // Liberación de memoria
    void internal_clear(Node* pNode);

    // Impresión inorder
    void internal_print(Node* pNode);

    // =========================
    // TAREA: size()
    // =========================
    size_t internal_size(Node* pNode) const;

    // =========================
    // TAREA: search()
    // =========================
    bool internal_search(Node* pNode,
                         const value_type& value) const;

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

    // =========================
    // TAREA: obtener tamaño
    // =========================
    size_t size() const {
        return internal_size(m_pRoot);
    }

    // =========================
    // TAREA: buscar elemento
    // =========================
    bool search(const value_type& value) const {
        return internal_search(m_pRoot, value);
    }

    // =========================
    // root: getter publico para composicion
    // Permite que clases que CONTIENEN un BinaryTree/AVLTree
    // (como HashMap) accedan a la raiz sin romper encapsulamiento
    // =========================
    Node* root() const {
        return m_pRoot;
    }

    // =========================
    // take_root: extrae la raiz dejando el arbol vacio.
    // Usado por contenedores que COMPONEN un arbol (HashMap)
    // para implementar su propio move constructor/assignment
    // sin duplicar punteros (evita doble free).
    // =========================
    Node* take_root(){
        Node* old = m_pRoot;
        m_pRoot = nullptr;
        return old;
    }

    // =========================
    // adopt_root: reemplaza la raiz actual (liberando la
    // anterior) por una ya existente. Complemento de
    // take_root() para mover el contenido entre arboles.
    // =========================
    void adopt_root(Node* newRoot){
        internal_clear(m_pRoot);
        m_pRoot = newRoot;
    }
};

// =====================================================
// IMPLEMENTACIONES
// =====================================================

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
        pNode->child(branch),
        data,
        ref
    );
}

template<typename Trait>
void BinaryTree<Trait>::internal_clear(Node* pNode){

    if(pNode == nullptr)
        return;

    internal_clear(pNode->child(0));
    internal_clear(pNode->child(1));

    delete pNode;
}

template<typename Trait>
void BinaryTree<Trait>::internal_print(Node* pNode){

    if(pNode == nullptr)
        return;

    internal_print(pNode->child(0));

    cout << "("
         << pNode->m_data
         << ","
         << pNode->m_ref
         << ") ";

    internal_print(pNode->child(1));
}

// =====================================================
// TAREA: SIZE
// =====================================================

template<typename Trait>
size_t BinaryTree<Trait>::internal_size(Node* pNode) const {

    if(pNode == nullptr)
        return 0;

    return 1
         + internal_size(pNode->child(0))
         + internal_size(pNode->child(1));
}

// =====================================================
// TAREA: SEARCH
// =====================================================

template<typename Trait>
bool BinaryTree<Trait>::internal_search(
    Node* pNode,
    const value_type& value) const
{
    if(pNode == nullptr)
        return false;

    if(pNode->m_data == value)
        return true;

    auto branch = !m_comp(value, pNode->m_data);

    return internal_search(
        pNode->child(branch),
        value
    );
}

#endif // __BINARYTREE_H__