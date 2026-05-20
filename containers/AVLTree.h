#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include "BinaryTree.h"

template<typename T>
class AVLNode : public BinaryTreeNode<T> {

public:
    using Node = AVLNode<T>;

private:
    int m_height;

public:

    AVLNode(T data, Ref ref)
        : BinaryTreeNode<T>(data, ref),
          m_height(1) {}

    int getHeight() const {
        return m_height;
    }

    void setHeight(int h) {
        m_height = h;
    }
};

template<typename T>
struct AscendingAVLTrait :
    public BaseTrait<
        AVLNode<T>,
        less<T>> {
};

template<typename T>
struct DescendingAVLTrait :
    public BaseTrait<
        AVLNode<T>,
        greater<T>> {
};

template<typename Trait>
class AVLTree : public BinaryTree<Trait> {

public:

    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

private:

    Node* m_pRoot = nullptr;

    Comp m_comp;

private:

    int height(Node* node) {

        if(node == nullptr)
            return 0;

        return node->getHeight();
    }

    int getBalance(Node* node) {

        if(node == nullptr)
            return 0;

        return height(
            static_cast<Node*>(node->m_pChild[0])
        ) - height(
            static_cast<Node*>(node->m_pChild[1])
        );
    }

    void updateHeight(Node* node) {

        int leftH = height(
            static_cast<Node*>(node->m_pChild[0])
        );

        int rightH = height(
            static_cast<Node*>(node->m_pChild[1])
        );

        node->setHeight(
            1 + max(leftH, rightH)
        );
    }

    Node* rotateRight(Node* y) {

        Node* x =
            static_cast<Node*>(y->m_pChild[0]);

        Node* T2 =
            static_cast<Node*>(x->m_pChild[1]);

        x->m_pChild[1] = y;
        y->m_pChild[0] = T2;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    Node* rotateLeft(Node* x) {

        Node* y =
            static_cast<Node*>(x->m_pChild[1]);

        Node* T2 =
            static_cast<Node*>(y->m_pChild[0]);

        y->m_pChild[0] = x;
        x->m_pChild[1] = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    Node* internal_insert(Node* node,
                          const value_type& value,
                          Ref ref) {

        if(node == nullptr)
            return new Node(value, ref);

        bool branch =
            m_comp(node->getData(), value);

        node->m_pChild[branch] =
            internal_insert(
                static_cast<Node*>(
                    node->m_pChild[branch]
                ),
                value,
                ref
            );

        updateHeight(node);

        int balance = getBalance(node);

        // LEFT LEFT
        if(balance > 1 &&
           m_comp(
               value,
               static_cast<Node*>(
                   node->m_pChild[0]
               )->getData()
           )) {

            return rotateRight(node);
        }

        // RIGHT RIGHT
        if(balance < -1 &&
           m_comp(
               static_cast<Node*>(
                   node->m_pChild[1]
               )->getData(),
               value
           )) {

            return rotateLeft(node);
        }

        // LEFT RIGHT
        if(balance > 1 &&
           m_comp(
               static_cast<Node*>(
                   node->m_pChild[0]
               )->getData(),
               value
           )) {

            node->m_pChild[0] =
                rotateLeft(
                    static_cast<Node*>(
                        node->m_pChild[0]
                    )
                );

            return rotateRight(node);
        }

        // RIGHT LEFT
        if(balance < -1 &&
           m_comp(
               value,
               static_cast<Node*>(
                   node->m_pChild[1]
               )->getData()
           )) {

            node->m_pChild[1] =
                rotateRight(
                    static_cast<Node*>(
                        node->m_pChild[1]
                    )
                );

            return rotateLeft(node);
        }

        return node;
    }

    void internal_destroy(Node* node) {

        if(node == nullptr)
            return;

        internal_destroy(
            static_cast<Node*>(node->m_pChild[0])
        );

        internal_destroy(
            static_cast<Node*>(node->m_pChild[1])
        );

        delete node;
    }

    void internal_inorder(Node* node,
                          ostream& os) const {

        if(node == nullptr)
            return;

        internal_inorder(
            static_cast<Node*>(node->m_pChild[0]),
            os
        );

        os << node->getData() << " ";

        internal_inorder(
            static_cast<Node*>(node->m_pChild[1]),
            os
        );
    }

public:

    AVLTree() = default;

    ~AVLTree() {

        internal_destroy(m_pRoot);
    }

    void insert(const value_type& value,
                Ref ref) {

        m_pRoot =
            internal_insert(
                m_pRoot,
                value,
                ref
            );
    }

    string toString() const {

        ostringstream os;

        os << "[ ";

        internal_inorder(m_pRoot, os);

        os << "]";

        return os.str();
    }

    friend ostream& operator<<(ostream& os,
                               const AVLTree& tree) {

        os << tree.toString();

        return os;
    }
};

#endif