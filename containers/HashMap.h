#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include "AVLTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// =====================================================
// Byte: tipo dedicado para parsing de caracteres
// de control en operator>>. No se usa 'char' crudo.
// =====================================================

using Byte = char;

// =====================================================
// MapLess: comparador genérico para pares (K,V),
// ordena solo por la clave K.
// No revela detalles de la implementación interna
// (AVL es un detalle de AscendingMapTrait, no del nombre).
// =====================================================

template<typename K, typename V>
struct MapLess {
    bool operator()(const pair<K,V>& a, const pair<K,V>& b) const {
        return a.first < b.first;
    }
};

// =====================================================
// AscendingMapTrait: Trait estandarizado para HashMap,
// mismo patrón que AscendingLinkedListTrait y AscendingTrait.
// No expone que la implementación interna usa AVL.
//
// Expone: value_type, Node, Comp
//
// Utilizar:
//   HashMap< AscendingMapTrait<K,V> > m;
// =====================================================

template<typename K, typename V>
struct AscendingMapTrait {
    using value_type = pair<K,V>;
    using Node       = AVLTreeNode< pair<K,V> >;
    using Comp       = MapLess<K,V>;
};

// =====================================================
// HashMap: mapa estandarizado, recibe Trait
// (mismo patrón que LinkedList<Trait> y BinaryTree<Trait>)
//
// La implementación interna (AVL) es un detalle privado;
// el Trait solo expone value_type, Node y Comp.
//
// Uso:
//   HashMap< AscendingMapTrait<T1,T1> > m;
//   m[5] = 3;
//   for (const auto& [key, value] : m) { ... }
//   cout << m;
//   cin  >> m;
// =====================================================

template<typename Trait>
class HashMap {
public:
    using value_type  = typename Trait::value_type;   // pair<K,V>
    using Node        = typename Trait::Node;
    using Comp        = typename Trait::Comp;
    using key_type     = typename value_type::first_type;
    using mapped_type  = typename value_type::second_type;
    using MySelf       = HashMap<Trait>;

private:
    // Estructura interna: árbol balanceado.
    // Implementación oculta tras la interfaz pública del HashMap.
    AVLTree<Trait>        m_tree;
    mutable shared_mutex  m_mtx;

    // =====================================================
    // root: delega en el getter publico de AVLTree
    // (ya no requiere static_cast gracias al CRTP en
    // BinaryTreeNode/AVLTreeNode)
    // =====================================================
    Node* root() const {
        return m_tree.root();
    }

    // =====================================================
    // internal_copy: recorre inorder e inserta en m_tree
    // =====================================================
    void internal_copy(Node* pNode) {
        if (pNode == nullptr) return;
        internal_copy(pNode->child(0));
        m_tree.insert(pNode->m_data, pNode->m_ref);
        internal_copy(pNode->child(1));
    }

    // =====================================================
    // clear: reinicia m_tree limpiamente
    // =====================================================
    void clear() {
        m_tree = AVLTree<Trait>();
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
    HashMap(HashMap&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_tree.adopt_root(other.m_tree.take_root());
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
    HashMap& operator=(HashMap&& other) noexcept {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_tree.adopt_root(other.m_tree.take_root());
        }
        return *this;
    }

    // =====================================================
    // Destructor
    // =====================================================
    virtual ~HashMap() {
        delete[] m_iter_arr;
    }

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
    // size: reutiliza size() del árbol interno
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
    // toString: reutiliza internal_toString del árbol interno
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
    // Usa Byte en vez de char crudo para los delimitadores
    // =====================================================
    friend istream& operator>>(istream& is, HashMap& map) {
        Byte b;

        if (!(is >> b) || b != '{') {
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
            Byte        colon;

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
// Usa T1 (definido en types.h) en vez de int crudo,
// igual que LinkedListDemo usa T1.
// =====================================================

void DemoHashMap(){

    cout << "\nTEST HASHMAP (AVL)" << endl;

    HashMap< AscendingMapTrait<T1,T1> > m;

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
    HashMap< AscendingMapTrait<T1,T1> > m2;
    stringstream ss("{10:20,30:40,50:60}");
    ss >> m2;
    cout << "Mapa leido con operator>>: " << m2 << endl;

    // Copy Constructor
    HashMap< AscendingMapTrait<T1,T1> > copia(m);
    cout << "Copy Constructor: " << copia << endl;

    // Move Constructor
    HashMap< AscendingMapTrait<T1,T1> > movido(std::move(copia));
    cout << "Move Constructor: " << movido << endl;

    cout << "Size: " << m.size() << endl;
}

#endif // __HASHMAP_H__