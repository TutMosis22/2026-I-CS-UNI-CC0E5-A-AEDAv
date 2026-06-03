#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <iostream>
#include <cstddef>   // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <functional>
#include <shared_mutex>
#include "../types.h"
using namespace std;

// =====================================================
// AVLNode: Nodo del árbol AVL
// Almacena un par (key, value) y la altura del nodo
// =====================================================

template <typename K, typename V>
struct AVLNode {
    using key_type    = K;
    using mapped_type = V;

    K           m_key;
    V           m_value;
    int         m_height;
    AVLNode*    m_pChild[2]; // [0] = izquierda, [1] = derecha

    AVLNode(const K& key, const V& value)
        : m_key(key),
          m_value(value),
          m_height(1),
          m_pChild{nullptr, nullptr}
    {}
};

// =====================================================
// HashMap: Mapa basado en árbol AVL
//
// Uso esperado (igual que std::map):
//   HashMap<int, int> m;
//   m[5] = 3;
//   for (const auto& [key, value] : m) { ... }
//   cout << m;
//   cin  >> m;
// =====================================================

template <typename K, typename V>
class HashMap {
public:
    using key_type    = K;
    using mapped_type = V;
    using value_type  = pair<const K, V>;
    using MySelf      = HashMap<K, V>;
    using Node        = AVLNode<K, V>;

private:
    Node*  m_pRoot;
    size_t m_size;
    mutable shared_mutex m_mtx;

    // -------------------------
    // Helpers AVL internos
    // -------------------------
    int    height(Node* p) const;
    int    balance_factor(Node* p) const;
    void   update_height(Node* p);
    Node*  rotate_right(Node* y);
    Node*  rotate_left(Node* x);
    Node*  rebalance(Node* p);

    // -------------------------
    // Operaciones internas
    // -------------------------
    Node*  internal_insert(Node* p, const K& key, const V& value, Node*& inserted);
    Node*  internal_find(Node* p, const K& key) const;
    Node*  internal_copy(Node* p) const;
    void   internal_clear(Node* p);
    void   internal_print(Node* p, ostream& os, bool& first) const;
    void   internal_to_sorted(Node* p, Node** arr, size_t& idx) const;
    size_t internal_size(Node* p) const;

public:

    // =====================================================
    // Constructor por defecto
    // =====================================================
    HashMap()
        : m_pRoot(nullptr), m_size(0)
    {}

    // =====================================================
    // Copy Constructor
    // =====================================================
    HashMap(const HashMap& other)
        : m_pRoot(nullptr), m_size(0)
    {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
        m_size  = other.m_size;
    }

    // =====================================================
    // Move Constructor
    // =====================================================
    HashMap(HashMap&& other)
        : m_pRoot(nullptr), m_size(0)
    {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }

    // =====================================================
    // Copy Assignment
    // =====================================================
    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            internal_clear(m_pRoot);
            m_pRoot = nullptr;
            m_size  = 0;
            shared_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = internal_copy(other.m_pRoot);
            m_size  = other.m_size;
        }
        return *this;
    }

    // =====================================================
    // Move Assignment
    // =====================================================
    HashMap& operator=(HashMap&& other) {
        if (this != &other) {
            internal_clear(m_pRoot);
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // =====================================================
    // Destructor
    // =====================================================
    virtual ~HashMap() {
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
        m_size  = 0;
    }

    // =====================================================
    // operator[]: Acceso/inserción por clave
    // Permite: m[5] = 3;
    // =====================================================
    V& operator[](const K& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* inserted = nullptr;
        m_pRoot = internal_insert(m_pRoot, key, V(), inserted);
        return inserted->m_value;
    }

    // =====================================================
    // insert: Inserción explícita
    // =====================================================
    void insert(const K& key, const V& value) {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* inserted = nullptr;
        m_pRoot = internal_insert(m_pRoot, key, value, inserted);
    }

    // =====================================================
    // find: Buscar por clave, retorna puntero al valor
    //       o nullptr si no existe
    // =====================================================
    V* find(const K& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* n = internal_find(m_pRoot, key);
        if (n == nullptr) return nullptr;
        return &(n->m_value);
    }

    // =====================================================
    // size: Cantidad de elementos
    // =====================================================
    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    // =====================================================
    // isEmpty
    // =====================================================
    bool isEmpty() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size == 0;
    }

    // =====================================================
    // toString: Formato { key:value, key:value, ... }
    // =====================================================
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        bool first = true;
        oss << "{";
        internal_print(m_pRoot, oss, first);
        oss << "}";
        return oss.str();
    }

    // =====================================================
    // operator<<
    // Formato: {key:value,key:value,...}
    // =====================================================
    friend ostream& operator<<(ostream& os, const HashMap& map) {
        return os << map.toString();
    }

    // =====================================================
    // operator>>
    // Formato esperado: {key:value,key:value,...}
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
            K key;
            V value;
            char colon;

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
    // Iterador para range-based for:
    // for (const auto& [key, value] : m) { ... }
    // =====================================================

    class iterator {
    private:
        Node**  m_arr;      // arreglo de punteros en orden inorder
        size_t  m_size;
        size_t  m_index;
    public:
        using value_type = pair<const K&, V&>;

        iterator(Node** arr, size_t size, size_t index)
            : m_arr(arr), m_size(size), m_index(index)
        {}

        // Structured bindings: devuelve pair de referencias
        pair<const K&, V&> operator*() const {
            return { m_arr[m_index]->m_key,
                     m_arr[m_index]->m_value };
        }

        iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return m_index != other.m_index;
        }
    };

    // =====================================================
    // const_iterator para range-based for con const
    // =====================================================

    class const_iterator {
    private:
        Node**  m_arr;
        size_t  m_size;
        size_t  m_index;
    public:
        const_iterator(Node** arr, size_t size, size_t index)
            : m_arr(arr), m_size(size), m_index(index)
        {}

        pair<const K&, const V&> operator*() const {
            return { m_arr[m_index]->m_key,
                     m_arr[m_index]->m_value };
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
    // Arreglo auxiliar para iteración inorder (se reconstruye en begin())
    // Se almacena como mutable para poder usarlo en const begin()
    mutable Node** m_iter_arr  = nullptr;
    mutable size_t m_iter_size = 0;

    void rebuild_iter_arr() const {
        delete[] m_iter_arr;
        m_iter_size = m_size;
        m_iter_arr  = new Node*[m_iter_size + 1];
        size_t idx  = 0;
        internal_to_sorted(m_pRoot, m_iter_arr, idx);
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
// IMPLEMENTACIONES PRIVADAS AVL
// =====================================================

template <typename K, typename V>
int HashMap<K,V>::height(Node* p) const {
    return p ? p->m_height : 0;
}

template <typename K, typename V>
int HashMap<K,V>::balance_factor(Node* p) const {
    if (!p) return 0;
    return height(p->m_pChild[0]) - height(p->m_pChild[1]);
}

template <typename K, typename V>
void HashMap<K,V>::update_height(Node* p) {
    if (!p) return;
    int lh = height(p->m_pChild[0]);
    int rh = height(p->m_pChild[1]);
    p->m_height = 1 + (lh > rh ? lh : rh);
}

// Rotación derecha: y es el nodo desbalanceado
template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::rotate_right(Node* y) {
    Node* x  = y->m_pChild[0];
    Node* T2 = x->m_pChild[1];

    x->m_pChild[1] = y;
    y->m_pChild[0] = T2;

    update_height(y);
    update_height(x);

    return x;
}

// Rotación izquierda: x es el nodo desbalanceado
template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::rotate_left(Node* x) {
    Node* y  = x->m_pChild[1];
    Node* T2 = y->m_pChild[0];

    y->m_pChild[0] = x;
    x->m_pChild[1] = T2;

    update_height(x);
    update_height(y);

    return y;
}

// Rebalanceo post-inserción
template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::rebalance(Node* p) {
    update_height(p);
    int bf = balance_factor(p);

    // Caso Left-Left
    if (bf > 1 && balance_factor(p->m_pChild[0]) >= 0)
        return rotate_right(p);

    // Caso Left-Right
    if (bf > 1 && balance_factor(p->m_pChild[0]) < 0) {
        p->m_pChild[0] = rotate_left(p->m_pChild[0]);
        return rotate_right(p);
    }

    // Caso Right-Right
    if (bf < -1 && balance_factor(p->m_pChild[1]) <= 0)
        return rotate_left(p);

    // Caso Right-Left
    if (bf < -1 && balance_factor(p->m_pChild[1]) > 0) {
        p->m_pChild[1] = rotate_right(p->m_pChild[1]);
        return rotate_left(p);
    }

    return p;
}

// Inserción AVL: si la clave ya existe, solo devuelve el nodo existente
template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::internal_insert(
    Node* p, const K& key, const V& value, Node*& inserted)
{
    if (p == nullptr) {
        p = new Node(key, value);
        inserted = p;
        m_size++;
        return p;
    }

    if (key < p->m_key) {
        p->m_pChild[0] = internal_insert(p->m_pChild[0], key, value, inserted);
    }
    else if (key > p->m_key) {
        p->m_pChild[1] = internal_insert(p->m_pChild[1], key, value, inserted);
    }
    else {
        // Clave duplicada: no insertar, devolver nodo existente
        inserted = p;
        return p;
    }

    return rebalance(p);
}

template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::internal_find(
    Node* p, const K& key) const
{
    if (p == nullptr) return nullptr;
    if (key == p->m_key) return p;
    if (key < p->m_key)  return internal_find(p->m_pChild[0], key);
    return                      internal_find(p->m_pChild[1], key);
}

template <typename K, typename V>
typename HashMap<K,V>::Node* HashMap<K,V>::internal_copy(Node* p) const {
    if (p == nullptr) return nullptr;
    Node* newNode = new Node(p->m_key, p->m_value);
    newNode->m_height    = p->m_height;
    newNode->m_pChild[0] = internal_copy(p->m_pChild[0]);
    newNode->m_pChild[1] = internal_copy(p->m_pChild[1]);
    return newNode;
}

template <typename K, typename V>
void HashMap<K,V>::internal_clear(Node* p) {
    if (p == nullptr) return;
    internal_clear(p->m_pChild[0]);
    internal_clear(p->m_pChild[1]);
    delete p;
}

// Impresión inorder: key:value, key:value, ...
template <typename K, typename V>
void HashMap<K,V>::internal_print(Node* p, ostream& os, bool& first) const {
    if (p == nullptr) return;
    internal_print(p->m_pChild[0], os, first);
    if (!first) os << ",";
    os << p->m_key << ":" << p->m_value;
    first = false;
    internal_print(p->m_pChild[1], os, first);
}

// Llenado inorder del arreglo de iteración
template <typename K, typename V>
void HashMap<K,V>::internal_to_sorted(Node* p, Node** arr, size_t& idx) const {
    if (p == nullptr) return;
    internal_to_sorted(p->m_pChild[0], arr, idx);
    arr[idx++] = p;
    internal_to_sorted(p->m_pChild[1], arr, idx);
}

template <typename K, typename V>
size_t HashMap<K,V>::internal_size(Node* p) const {
    if (p == nullptr) return 0;
    return 1 + internal_size(p->m_pChild[0]) + internal_size(p->m_pChild[1]);
}

// =====================================================
// DemoHashMap: función de demostración
// Llamar desde ListsDemo() o main()
// =====================================================
void DemoHashMap(){

    cout << "\nTEST HASHMAP (AVL)" << endl;

    HashMap<int, int> m;

    // operator[]: inserción y asignación
    m[5] = 10;
    m[3] = 6;
    m[8] = 16;
    m[1] = 2;
    m[4] = 8;

    cout << "Mapa inicial: " << m << endl;

    // Modificación por clave existente
    m[3] = 99;
    cout << "Luego de m[3] = 99: " << m << endl;

    // Range-based for con structured bindings
    cout << "Iteracion con for (const auto& [key, value] : m):" << endl;
    for (const auto& [key, value] : m) {
        cout << "  " << key << " -> " << value << endl;
    }

    // operator>>
    HashMap<int, int> m2;
    stringstream ss("{10:20,30:40,50:60}");
    ss >> m2;
    cout << "Mapa leido con operator>>: " << m2 << endl;

    // Copy Constructor
    HashMap<int, int> copia(m);
    cout << "Copy Constructor: " << copia << endl;

    // Move Constructor
    HashMap<int, int> movido(std::move(copia));
    cout << "Move Constructor: " << movido << endl;

    cout << "Size: " << m.size() << endl;
}

#endif // __HASHMAP_H__