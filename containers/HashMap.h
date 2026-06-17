#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <utility>
#include <shared_mutex>
#include "AVLTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// =====================================================
// KeyLess: comparador para el AVL interno del HashMap
// Compara pares (K,V) solo por la clave K
// =====================================================

template<typename K, typename V>
struct KeyLess {
    bool operator()(const pair<K,V>& a, const pair<K,V>& b) const {
        return a.first < b.first;
    }
};

// =====================================================
// HashMapTrait: Trait estandarizado para HashMap,
// siguiendo el mismo patrón que AscendingLinkedListTrait
// y AscendingTrait usados en LinkedList y BinaryTree.
//
// Expone: value_type, Node, Comp
//
// Utilizar:
//   HashMap< HashMapTrait<K,V> > m;
// =====================================================

template<typename K, typename V>
struct HashMapTrait {
    using value_type = pair<K,V>;
    using Node       = AVLTreeNode< pair<K,V> >;
    using Comp       = KeyLess<K,V>;
};

// =====================================================
// HashMap: mapa basado en AVL, recibe Trait
// (mismo patrón que LinkedList<Trait> y BinaryTree<Trait>)
//
// Internamente tiene:
//   AVLTree<Trait> m_tree;
//
// Uso:
//   HashMap< HashMapTrait<int,int> > m;
//   m[5] = 3;
//   for (const auto& [key, value] : m) { ... }
//   cout << m;
//   cin  >> m;
// =====================================================

template<typename Trait>
class HashMap {
public:
    using value_type = typename Trait::value_type;   // pair<K,V>
    using Node       = typename Trait::Node;          // AVLTreeNode<pair<K,V>>
    using Comp       = typename Trait::Comp;          // KeyLess<K,V>
    using key_type    = typename value_type::first_type;
    using mapped_type = typename value_type::second_type;
    using MySelf      = HashMap<Trait>;
    using Tree        = AVLTree<Trait>;

private:
    Tree                 m_tree;
    mutable shared_mutex m_mtx;

    // =====================================================
    // child / root: acceso tipado a nodos
    // Encapsula el cast en un solo lugar..
    // =====================================================
    Node* child(Node* p, int dir) const {
        return static_cast<Node*>(p->m_pChild[dir]);
    }

    Node* root() const {
        return static_cast<Node*>(m_tree.m_pRoot);
    }

    // =====================================================
    // internal_copy: recorre inorder e inserta en m_tree
    // =====================================================
    void internal_copy(Node* pNode) {
        if (pNode == nullptr) return;
        internal_copy(child(pNode, 0));
        m_tree.insert(pNode->m_data, pNode->m_ref);
        internal_copy(child(pNode, 1));
    }

    // =====================================================
    // clear: reinicia m_tree limpiamente
    // =====================================================
    void clear() {
        m_tree = Tree();
    }

public:

    // =====================================================
    // Constructor por defecto
    // =====================================================
    HashMap() {}

    // =====================================================
    // Copy Constructor
    // =====================================================
    HashMap(const HashMap& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        internal_copy(other.root());
    }

    // =====================================================
    // Move Constructor
    // =====================================================
    HashMap(HashMap&& other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_tree.m_pRoot = std::exchange(other.m_tree.m_pRoot, nullptr);
    }

    // =====================================================
    // Copy Assignment
    // =====================================================
    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            internal_copy(other.root());
        }
        return *this;
    }

    // =====================================================
    // Move Assignment
    // =====================================================
    HashMap& operator=(HashMap&& other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_tree.m_pRoot = std::exchange(other.m_tree.m_pRoot, nullptr);
        }
        return *this;
    }

    // =====================================================
    // Destructor
    // =====================================================
    virtual ~HashMap() {}

    // =====================================================
    // operator[]: acceso/inserción por clave
    // Permite: m[5] = 3;
    // =====================================================
    mapped_type& operator[](const key_type& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        value_type probe(key, mapped_type());
        Node* node = m_tree.find(probe);
        if (node == nullptr) {
            m_tree.insert(probe, 0);
            node = m_tree.find(probe);
        }
        return node->m_data.second;
    }

    // =====================================================
    // insert: inserción explícita
    // =====================================================
    void insert(const key_type& key, const mapped_type& value) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_tree.insert(value_type(key, value), 0);
    }

    // =====================================================
    // find: retorna puntero al valor o nullptr
    // =====================================================
    mapped_type* find(const key_type& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* node = m_tree.find(value_type(key, mapped_type()));
        if (node == nullptr) return nullptr;
        return &(node->m_data.second);
    }

    // =====================================================
    // size: reutiliza size() del AVL
    // =====================================================
    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_tree.size();
    }

    // =====================================================
    // isEmpty
    // =====================================================
    bool isEmpty() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_tree.size() == 0;
    }

    // =====================================================
    // toString: reutiliza internal_toString del AVL
    // Formato: {key:value,key:value,...}
    // =====================================================
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        bool first = true;
        oss << "{";
        m_tree.internal_toString(root(), oss, first);
        oss << "}";
        return oss.str();
    }

    // =====================================================
    // operator<<
    // =====================================================
    friend ostream& operator<<(ostream& os, const HashMap& map) {
        return os << map.toString();
    }

    // =====================================================
    // operator>>
    // Formato: {key:value,key:value,...}
    // =====================================================
    friend istream& operator>>(istream& is, HashMap& map) {
        char ch;

        if (!(is >> ch) || ch != '{') {
            is.setstate(ios::failbit);
            return is;
        }

        is >> ws;

        if (is.peek() == '}') {
            is.get();
            return is;
        }

        while (true) {
            key_type    key;
            mapped_type value;
            char        colon;

            if (!(is >> key)) {
                is.setstate(ios::failbit);
                return is;
            }

            if (!(is >> colon) || colon != ':') {
                is.setstate(ios::failbit);
                return is;
            }

            if (!(is >> value)) {
                is.setstate(ios::failbit);
                return is;
            }

            map.insert(key, value);

            is >> ws;

            if (is.peek() == ',') {
                is.get();
                is >> ws;
            }
            else if (is.peek() == '}') {
                is.get();
                break;
            }
            else {
                is.setstate(ios::failbit);
                return is;
            }
        }

        return is;
    }

    // =====================================================
    // Iterador: recorre m_tree inorder
    // Permite: for (const auto& [key, value] : m)
    // =====================================================

    class iterator {
    private:
        value_type** m_arr;
        size_t       m_size;
        size_t       m_index;
    public:
        iterator(value_type** arr, size_t size, size_t index)
            : m_arr(arr), m_size(size), m_index(index)
        {}

        pair<const key_type&, mapped_type&> operator*() const {
            return { m_arr[m_index]->first,
                     m_arr[m_index]->second };
        }

        iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return m_index != other.m_index;
        }
    };

    class const_iterator {
    private:
        value_type** m_arr;
        size_t       m_size;
        size_t       m_index;
    public:
        const_iterator(value_type** arr, size_t size, size_t index)
            : m_arr(arr), m_size(size), m_index(index)
        {}

        pair<const key_type&, const mapped_type&> operator*() const {
            return { m_arr[m_index]->first,
                     m_arr[m_index]->second };
        }

        const_iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator!=(const const_iterator& other) const {
            return m_index != other.m_index;
        }
    };

private:
    mutable value_type** m_iter_arr  = nullptr;
    mutable size_t       m_iter_size = 0;

    void rebuild_iter_arr() const {
        delete[] m_iter_arr;
        m_iter_size = m_tree.size();
        m_iter_arr  = new value_type*[m_iter_size + 1];
        size_t idx  = 0;
        m_tree.internal_collect(root(), m_iter_arr, idx);
    }

public:
    iterator begin() {
        shared_lock<shared_mutex> lock(m_mtx);
        rebuild_iter_arr();
        return iterator(m_iter_arr, m_iter_size, 0);
    }

    iterator end() {
        return iterator(m_iter_arr, m_iter_size, m_iter_size);
    }

    const_iterator begin() const {
        shared_lock<shared_mutex> lock(m_mtx);
        rebuild_iter_arr();
        return const_iterator(m_iter_arr, m_iter_size, 0);
    }

    const_iterator end() const {
        return const_iterator(m_iter_arr, m_iter_size, m_iter_size);
    }
};

// =====================================================
// DemoHashMap
// =====================================================

void DemoHashMap(){

    cout << "\nTEST HASHMAP (AVL)" << endl;

    HashMap< HashMapTrait<int,int> > m;

    // operator[]: inserción y asignación
    m[5] = 10;
    m[3] = 6;
    m[8] = 16;
    m[1] = 2;
    m[4] = 8;

    cout << "Mapa inicial: " << m << endl;

    // Modificación de clave existente
    m[3] = 99;
    cout << "Luego de m[3] = 99: " << m << endl;

    // Range-based for con structured bindings
    cout << "Iteracion con for (const auto& [key, value] : m):" << endl;
    for (const auto& [key, value] : m) {
        cout << "  " << key << " -> " << value << endl;
    }

    // operator>>
    HashMap< HashMapTrait<int,int> > m2;
    stringstream ss("{10:20,30:40,50:60}");
    ss >> m2;
    cout << "Mapa leido con operator>>: " << m2 << endl;

    // Copy Constructor
    HashMap< HashMapTrait<int,int> > copia(m);
    cout << "Copy Constructor: " << copia << endl;

    // Move Constructor
    HashMap< HashMapTrait<int,int> > movido(std::move(copia));
    cout << "Move Constructor: " << movido << endl;

    cout << "Size: " << m.size() << endl;
}

#endif // __HASHMAP_H__