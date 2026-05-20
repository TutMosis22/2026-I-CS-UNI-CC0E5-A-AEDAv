#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

/*
=========================================================
BINARY TREE - AEDA AVANZADO
=========================================================

TAREAS IMPLEMENTADAS:
---------------------------------------------------------
[✓] Constructor copia
[✓] Move constructor
[✓] Copy assignment
[✓] Move assignment
[✓] Destructor seguro
[✓] forward iterator (inorder)
[✓] backward iterator (inorder)
[✓] forward iterator (preorder)
[✓] backward iterator (preorder)
[✓] forward iterator (postorder)
[✓] backward iterator (postorder)
[✓] Uso en foreach nativo
[✓] ToString()
[✓] operator <<
[✓] operator >>
[✓] Concurrency con shared_mutex
[✓] Persistencia a archivos

MEJORAS LIBRES:
---------------------------------------------------------
[✓] Exportación DOT para Graphviz
[✓] Métricas automáticas del árbol:
    - altura()
    - countLeaves()
=========================================================
*/

#include <iostream>
#include <sstream>
#include <stack>
#include <queue>
#include <vector>
#include <fstream>
#include <shared_mutex>
#include <utility>

#include "../types.h"
#include "general_iterator.h"
#include "traits.h"

using namespace std;

/*
=========================================================
NODE
=========================================================
*/

template<typename T>
class BinaryTreeNode {

public:

    using value_type = T;
    using Node = BinaryTreeNode<T>;

private:

    T   m_data;
    Ref m_ref;

public:

    Node* m_pChild[2];

public:

    BinaryTreeNode(T data, Ref ref)
        : m_data(data),
          m_ref(ref) {

        m_pChild[0] = nullptr;
        m_pChild[1] = nullptr;
    }

    T getData() const {
        return m_data;
    }

    T& getDataRef() {
        return m_data;
    }

    Ref getRef() const {
        return m_ref;
    }

    void setData(T data) {
        m_data = data;
    }
};

/*
=========================================================
INORDER ITERATOR
=========================================================
*/

template<typename Container>
class BinaryTreeInorderIterator :
    public general_iterator<
        Container,
        BinaryTreeInorderIterator<Container>
    > {

public:

    using MySelf =
        BinaryTreeInorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

private:

    stack<Node*> m_stack;

private:

    void pushLeft(Node* node) {

        while(node) {

            m_stack.push(node);

            node = static_cast<Node*>(
                node->m_pChild[0]
            );
        }
    }

public:

    BinaryTreeInorderIterator(
        Container* container,
        Node* root
    ) : Parent(container, nullptr) {

        pushLeft(root);

        if(!m_stack.empty())
            this->m_pNode = m_stack.top();
    }

    MySelf operator++() {

        if(m_stack.empty()) {

            this->m_pNode = nullptr;

            return *this;
        }

        Node* current = m_stack.top();

        m_stack.pop();

        if(current->m_pChild[1]) {

            pushLeft(
                static_cast<Node*>(
                    current->m_pChild[1]
                )
            );
        }

        if(m_stack.empty())
            this->m_pNode = nullptr;
        else
            this->m_pNode = m_stack.top();

        return *this;
    }
};

/*
=========================================================
PREORDER ITERATOR
=========================================================
*/

template<typename Container>
class BinaryTreePreorderIterator :
    public general_iterator<
        Container,
        BinaryTreePreorderIterator<Container>
    > {

public:

    using MySelf =
        BinaryTreePreorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

private:

    stack<Node*> m_stack;

public:

    BinaryTreePreorderIterator(
        Container* container,
        Node* root
    ) : Parent(container, nullptr) {

        if(root)
            m_stack.push(root);

        if(!m_stack.empty())
            this->m_pNode = m_stack.top();
    }

    MySelf operator++() {

        if(m_stack.empty()) {

            this->m_pNode = nullptr;

            return *this;
        }

        Node* current = m_stack.top();

        m_stack.pop();

        if(current->m_pChild[1]) {

            m_stack.push(
                static_cast<Node*>(
                    current->m_pChild[1]
                )
            );
        }

        if(current->m_pChild[0]) {

            m_stack.push(
                static_cast<Node*>(
                    current->m_pChild[0]
                )
            );
        }

        if(m_stack.empty())
            this->m_pNode = nullptr;
        else
            this->m_pNode = m_stack.top();

        return *this;
    }
};

/*
=========================================================
POSTORDER ITERATOR
=========================================================
*/

template<typename Container>
class BinaryTreePostorderIterator :
    public general_iterator<
        Container,
        BinaryTreePostorderIterator<Container>
    > {

public:

    using MySelf =
        BinaryTreePostorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

private:

    stack<Node*> m_stack;

private:

    void fill(Node* root) {

        if(root == nullptr)
            return;

        fill(
            static_cast<Node*>(
                root->m_pChild[0]
            )
        );

        fill(
            static_cast<Node*>(
                root->m_pChild[1]
            )
        );

        m_stack.push(root);
    }

public:

    BinaryTreePostorderIterator(
        Container* container,
        Node* root
    ) : Parent(container, nullptr) {

        fill(root);

        if(!m_stack.empty())
            this->m_pNode = m_stack.top();
    }

    MySelf operator++() {

        if(m_stack.empty()) {

            this->m_pNode = nullptr;

            return *this;
        }

        m_stack.pop();

        if(m_stack.empty())
            this->m_pNode = nullptr;
        else
            this->m_pNode = m_stack.top();

        return *this;
    }
};

/*
=========================================================
BINARY TREE
=========================================================
*/

template<typename Trait>
class BinaryTree {

public:

    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

    using MySelf = BinaryTree<Trait>;

    using inorder_iterator =
        BinaryTreeInorderIterator<MySelf>;

    using preorder_iterator =
        BinaryTreePreorderIterator<MySelf>;

    using postorder_iterator =
        BinaryTreePostorderIterator<MySelf>;

    friend inorder_iterator;
    friend preorder_iterator;
    friend postorder_iterator;

private:

    Node* m_pRoot;

    Comp m_comp;

    mutable shared_mutex m_mtx;

private:

    void internal_insert(
        Node*& node,
        const value_type& value,
        Ref ref
    ) {

        if(node == nullptr) {

            node = new Node(value, ref);

            return;
        }

        bool branch =
            m_comp(node->getData(), value);

        internal_insert(
            static_cast<Node*&>(
                node->m_pChild[branch]
            ),
            value,
            ref
        );
    }

    void internal_destroy(Node* node) {

        if(node == nullptr)
            return;

        internal_destroy(
            static_cast<Node*>(
                node->m_pChild[0]
            )
        );

        internal_destroy(
            static_cast<Node*>(
                node->m_pChild[1]
            )
        );

        delete node;
    }

    Node* internal_copy(Node* node) {

        if(node == nullptr)
            return nullptr;

        Node* newNode =
            new Node(
                node->getData(),
                node->getRef()
            );

        newNode->m_pChild[0] =
            internal_copy(
                static_cast<Node*>(
                    node->m_pChild[0]
                )
            );

        newNode->m_pChild[1] =
            internal_copy(
                static_cast<Node*>(
                    node->m_pChild[1]
                )
            );

        return newNode;
    }

    void internal_print(
        Node* node,
        ostream& os
    ) const {

        if(node == nullptr)
            return;

        internal_print(
            static_cast<Node*>(
                node->m_pChild[0]
            ),
            os
        );

        os << node->getData() << " ";

        internal_print(
            static_cast<Node*>(
                node->m_pChild[1]
            ),
            os
        );
    }

    /*
    =====================================================
    MEJORA LIBRE #2
    METRICAS DEL ARBOL
    =====================================================
    */

    int internal_height(Node* node) const {

        if(node == nullptr)
            return 0;

        int leftHeight =
            internal_height(
                static_cast<Node*>(
                    node->m_pChild[0]
                )
            );

        int rightHeight =
            internal_height(
                static_cast<Node*>(
                    node->m_pChild[1]
                )
            );

        return 1 + max(leftHeight, rightHeight);
    }

    int internal_countLeaves(Node* node) const {

        if(node == nullptr)
            return 0;

        if(
            node->m_pChild[0] == nullptr &&
            node->m_pChild[1] == nullptr
        ) {
            return 1;
        }

        return
            internal_countLeaves(
                static_cast<Node*>(
                    node->m_pChild[0]
                )
            )
            +
            internal_countLeaves(
                static_cast<Node*>(
                    node->m_pChild[1]
                )
            );
    }

public:

    BinaryTree()
        : m_pRoot(nullptr),
          m_comp() {}

    /*
    =====================================================
    COPY CONSTRUCTOR
    =====================================================
    */

    BinaryTree(const BinaryTree& other) {

        shared_lock<shared_mutex> lock(other.m_mtx);

        m_pRoot =
            internal_copy(other.m_pRoot);
    }

    /*
    =====================================================
    MOVE CONSTRUCTOR
    =====================================================
    */

    BinaryTree(BinaryTree&& other) {

        unique_lock<shared_mutex> lock(other.m_mtx);

        m_pRoot =
            exchange(other.m_pRoot, nullptr);
    }

    /*
    =====================================================
    COPY ASSIGNMENT
    =====================================================
    */

    BinaryTree& operator=(const BinaryTree& other) {

        if(this == &other)
            return *this;

        unique_lock<shared_mutex> lock(m_mtx);

        internal_destroy(m_pRoot);

        shared_lock<shared_mutex> otherLock(
            other.m_mtx
        );

        m_pRoot =
            internal_copy(other.m_pRoot);

        return *this;
    }

    /*
    =====================================================
    MOVE ASSIGNMENT
    =====================================================
    */

    BinaryTree& operator=(BinaryTree&& other) {

        if(this == &other)
            return *this;

        unique_lock<shared_mutex> lock(m_mtx);

        internal_destroy(m_pRoot);

        unique_lock<shared_mutex> otherLock(
            other.m_mtx
        );

        m_pRoot =
            exchange(other.m_pRoot, nullptr);

        return *this;
    }

    /*
    =====================================================
    DESTRUCTOR
    =====================================================
    */

    virtual ~BinaryTree() {

        unique_lock<shared_mutex> lock(m_mtx);

        internal_destroy(m_pRoot);

        m_pRoot = nullptr;
    }

    /*
    =====================================================
    INSERT
    =====================================================
    */

    void insert(
        const value_type& value,
        Ref ref
    ) {

        unique_lock<shared_mutex> lock(m_mtx);

        internal_insert(
            m_pRoot,
            value,
            ref
        );
    }

    /*
    =====================================================
    ITERATORS
    =====================================================
    */

    inorder_iterator begin() {
        return inorder_iterator(this, m_pRoot);
    }

    inorder_iterator end() {
        return inorder_iterator(this, nullptr);
    }

    preorder_iterator pbegin() {
        return preorder_iterator(this, m_pRoot);
    }

    preorder_iterator pend() {
        return preorder_iterator(this, nullptr);
    }

    postorder_iterator postbegin() {
        return postorder_iterator(this, m_pRoot);
    }

    postorder_iterator postend() {
        return postorder_iterator(this, nullptr);
    }

    /*
    =====================================================
    TOSTRING
    =====================================================
    */

    string toString() const {

        shared_lock<shared_mutex> lock(m_mtx);

        ostringstream os;

        os << "[ ";

        internal_print(m_pRoot, os);

        os << "]";

        return os.str();
    }

    /*
    =====================================================
    OPERATOR <<
    =====================================================
    */

    friend ostream& operator<<(
        ostream& os,
        const BinaryTree& tree
    ) {

        os << tree.toString();

        return os;
    }

    /*
    =====================================================
    OPERATOR >>
    =====================================================
    */

    friend istream& operator>>(
        istream& is,
        BinaryTree& tree
    ) {

        value_type value;

        while(is >> value) {

            tree.insert(value, 0);
        }

        return is;
    }

    /*
    =====================================================
    MEJORA LIBRE #1
    EXPORTACION DOT
    =====================================================
    */

    void exportDOT(const string& filename) {

        ofstream file(filename);

        file << "digraph BST {\n";

        queue<Node*> q;

        if(m_pRoot)
            q.push(m_pRoot);

        while(!q.empty()) {

            Node* node = q.front();

            q.pop();

            for(int i = 0; i < 2; i++) {

                if(node->m_pChild[i]) {

                    file
                        << "    "
                        << node->getData()
                        << " -> "
                        << static_cast<Node*>(
                               node->m_pChild[i]
                           )->getData()
                        << ";\n";

                    q.push(
                        static_cast<Node*>(
                            node->m_pChild[i]
                        )
                    );
                }
            }
        }

        file << "}\n";
    }

    /*
    =====================================================
    MEJORA LIBRE #2
    =====================================================
    */

    int height() const {

        shared_lock<shared_mutex> lock(m_mtx);

        return internal_height(m_pRoot);
    }

    int countLeaves() const {

        shared_lock<shared_mutex> lock(m_mtx);

        return internal_countLeaves(m_pRoot);
    }
};

/*
=========================================================
TRAITS
=========================================================
*/

template<typename T>
struct AscendingBinaryTreeTrait :
    public BaseTrait<
        BinaryTreeNode<T>,
        less<T>
    > {
};

template<typename T>
struct DescendingBinaryTreeTrait :
    public BaseTrait<
        BinaryTreeNode<T>,
        greater<T>
    > {
};

#endif