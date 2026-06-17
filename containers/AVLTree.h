#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include <cstddef>
#include <sstream>
#include "BinaryTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// =====================================================
// Tipo unificado para altura
// =====================================================

using height_t = size_t;

// =====================================================
// operator<< para std::pair, necesario para que
// AVLTree::internal_toString pueda imprimir nodos cuyo
// value_type sea un pair<K,V> (caso usado por HashMap).
// Formato: key:value
// =====================================================

#include <utility>

template<typename K, typename V>
ostream& operator<<(ostream& os, const pair<K,V>& p){
    return os << p.first << ":" << p.second;
}

// =====================================================
// AVLTreeNode: extiende BinaryTreeNode con altura.
// CRTP: se pasa a si mismo como Derived, asi m_pChild
// ya es AVLTreeNode<T>*[2], sin necesidad de static_cast
// en ningun lugar de AVLTree.
// =====================================================

template<typename T>
struct AVLTreeNode : public BinaryTreeNode<T, AVLTreeNode<T>> {
    using value_type = T;
    using Base       = BinaryTreeNode<T, AVLTreeNode<T>>;

    height_t m_height;

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

    height_t height(Node* p) const {
        if (p == nullptr) return 0;
        return p->m_height;
    }

    void update_height(Node* p) {
        if (p == nullptr) return;
        height_t lh = height(p->child(0));
        height_t rh = height(p->child(1));
        p->m_height  = 1 + (lh > rh ? lh : rh);
    }

    // balance_factor: diferencia de alturas izq - der
    // positivo = pesado izquierda, negativo = pesado derecha
    // Se mantiene como long solo aquí para poder ser negativo
    long balance_factor(Node* p) const {
        if (p == nullptr) return 0;
        return (long)height(p->child(0))
             - (long)height(p->child(1));
    }

    // =====================================================
    // rotate: rotación unificada
    // dir = 0 → rotación derecha (subárbol izquierdo pesado)
    // dir = 1 → rotación izquierda (subárbol derecho pesado)
    //
    // El pivote es el hijo del lado pesado: p->child(dir).
    // Ese pivote sube, y su hijo opuesto (1-dir) pasa a ser
    // el nuevo hijo de p en el lado dir.
    // =====================================================
    Node* rotate(Node* p, int dir) {
        int   other   = 1 - dir;
        Node* pivot   = p->child(dir);
        Node* subtree = pivot->child(other);

        pivot->child(other) = p;
        p->child(dir)       = subtree;

        update_height(p);
        update_height(pivot);

        return pivot;
    }

    Node* rebalance(Node* p) {
        update_height(p);
        long bf = balance_factor(p);

        // Left-Left
        if (bf > 1 && balance_factor(p->child(0)) >= 0)
            return rotate(p, 0);

        // Left-Right
        if (bf > 1 && balance_factor(p->child(0)) < 0) {
            p->child(0) = rotate(p->child(0), 1);
            return rotate(p, 0);
        }

        // Right-Right
        if (bf < -1 && balance_factor(p->child(1)) <= 0)
            return rotate(p, 1);

        // Right-Left
        if (bf < -1 && balance_factor(p->child(1)) > 0) {
            p->child(1) = rotate(p->child(1), 0);
            return rotate(p, 1);
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
            pNode = new Node(data, ref);
            return;
        }

        if (this->m_comp(data, pNode->m_data)) {
            internal_insert(pNode->child(0), data, ref);
        }
        else if (this->m_comp(pNode->m_data, data)) {
            internal_insert(pNode->child(1), data, ref);
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
            return find(pNode->child(0), data);
        return find(pNode->child(1), data);
    }

    Node* find(const value_type& data) const {
        return find(this->root(), data);
    }

    // =====================================================
    // toString: recorrido inorder
    // Reutilizable por HashMap
    // =====================================================
    void internal_toString(Node* pNode,
                           ostringstream& oss,
                           bool& first) const {
        if (pNode == nullptr) return;
        internal_toString(pNode->child(0), oss, first);
        if (!first) oss << ",";
        oss << pNode->m_data;
        first = false;
        internal_toString(pNode->child(1), oss, first);
    }

    string toString() const {
        ostringstream oss;
        bool first = true;
        oss << "{";
        internal_toString(this->root(), oss, first);
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
        internal_collect(pNode->child(0), arr, idx);
        arr[idx++] = &(pNode->m_data);
        internal_collect(pNode->child(1), arr, idx);
    }
};

#endif // __AVLTREE_H__