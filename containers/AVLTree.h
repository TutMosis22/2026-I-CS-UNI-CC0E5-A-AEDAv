#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include <cstddef>
#include <sstream>
#include "BinaryTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// =====================================================
// AVLTreeNode: extiende BinaryTreeNode con altura
// =====================================================

template<typename T>
struct AVLTreeNode : public BinaryTreeNode<T> {
    using value_type = T;
    using Base       = BinaryTreeNode<T>;

    size_t m_height;

    AVLTreeNode(T data, Ref ref = Ref())
        : Base(data, ref),
          m_height(1)
    {}
};

// =====================================================
// AVLTree: hereda BinaryTree, sobreescribe insert
// con rebalanceo AVL
//
// Utilizar:
//   AscendingTrait< AVLTreeNode<T> >
//   DescendingTrait< AVLTreeNode<T> >
// =====================================================

template<typename Trait>
class AVLTree : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = AVLTree<Trait>;
    using Parent     = BinaryTree<Trait>;

protected:

    // -------------------------
    // Helpers AVL
    // -------------------------

    size_t height(Node* p) const {
        if (p == nullptr) return 0;
        return static_cast<AVLTreeNode<value_type>*>(p)->m_height;
    }

    void update_height(Node* p) {
        if (p == nullptr) return;
        size_t lh = height(p->m_pChild[0]);
        size_t rh = height(p->m_pChild[1]);
        static_cast<AVLTreeNode<value_type>*>(p)->m_height =
            1 + (lh > rh ? lh : rh);
    }

    // balance_factor: usa long para poder ser negativo
    long balance_factor(Node* p) const {
        if (p == nullptr) return 0;
        return (long)height(p->m_pChild[0])
             - (long)height(p->m_pChild[1]);
    }

    // Rotación derecha: y es el nodo desbalanceado
    Node* rotate_right(Node* y) {
        Node* x  = y->m_pChild[0];
        Node* T2 = x->m_pChild[1];

        x->m_pChild[1] = y;
        y->m_pChild[0] = T2;

        update_height(y);
        update_height(x);

        return x;
    }

    // Rotación izquierda: x es el nodo desbalanceado
    Node* rotate_left(Node* x) {
        Node* y  = x->m_pChild[1];
        Node* T2 = y->m_pChild[0];

        y->m_pChild[0] = x;
        x->m_pChild[1] = T2;

        update_height(x);
        update_height(y);

        return y;
    }

    Node* rebalance(Node* p) {
        update_height(p);
        long bf = balance_factor(p);

        // Left-Left
        if (bf > 1 && balance_factor(p->m_pChild[0]) >= 0)
            return rotate_right(p);

        // Left-Right
        if (bf > 1 && balance_factor(p->m_pChild[0]) < 0) {
            p->m_pChild[0] = rotate_left(p->m_pChild[0]);
            return rotate_right(p);
        }

        // Right-Right
        if (bf < -1 && balance_factor(p->m_pChild[1]) <= 0)
            return rotate_left(p);

        // Right-Left
        if (bf < -1 && balance_factor(p->m_pChild[1]) > 0) {
            p->m_pChild[1] = rotate_right(p->m_pChild[1]);
            return rotate_left(p);
        }

        return p;
    }

    // =====================================================
    // Override: inserción AVL con rebalanceo
    // Si la clave ya existe, actualiza el valor
    // =====================================================
    void internal_insert(Node*& pNode,
                         const value_type& data,
                         Ref ref) override {
        if (pNode == nullptr) {
            pNode = new AVLTreeNode<value_type>(data, ref);
            return;
        }

        if (this->m_comp(data, pNode->m_data)) {
            internal_insert(pNode->m_pChild[0], data, ref);
        }
        else if (this->m_comp(pNode->m_data, data)) {
            internal_insert(pNode->m_pChild[1], data, ref);
        }
        else {
            // Clave duplicada: actualizar
            pNode->m_data = data;
            return;
        }

        pNode = rebalance(pNode);
    }

public:

    AVLTree() : Parent() {}

    virtual ~AVLTree() {}

    // =====================================================
    // find: buscar nodo por valor, retorna puntero o nullptr
    // =====================================================
    Node* find(Node* pNode, const value_type& data) const {
        if (pNode == nullptr) return nullptr;
        if (!this->m_comp(data, pNode->m_data) &&
            !this->m_comp(pNode->m_data, data))
            return pNode;
        if (this->m_comp(data, pNode->m_data))
            return find(pNode->m_pChild[0], data);
        return find(pNode->m_pChild[1], data);
    }

    Node* find(const value_type& data) const {
        return find(this->m_pRoot, data);
    }

    // =====================================================
    // toString: recorrido inorder
    // Reutilizable por HashMap
    // =====================================================
    void internal_toString(Node* pNode,
                           ostringstream& oss,
                           bool& first) const {
        if (pNode == nullptr) return;
        internal_toString(pNode->m_pChild[0], oss, first);
        if (!first) oss << ",";
        oss << pNode->m_data;
        first = false;
        internal_toString(pNode->m_pChild[1], oss, first);
    }

    string toString() const {
        ostringstream oss;
        bool first = true;
        oss << "{";
        internal_toString(this->m_pRoot, oss, first);
        oss << "}";
        return oss.str();
    }

    // =====================================================
    // internal_collect: llena arreglo inorder para iterador
    // =====================================================
    void internal_collect(Node* pNode,
                          value_type** arr,
                          size_t& idx) const {
        if (pNode == nullptr) return;
        internal_collect(pNode->m_pChild[0], arr, idx);
        arr[idx++] = &(pNode->m_data);
        internal_collect(pNode->m_pChild[1], arr, idx);
    }
};

#endif // __AVLTREE_H__