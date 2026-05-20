#include <iostream>

#include "BinaryTree.h"
#include "AVLTree.h"

using namespace std;

/*
=========================================================
DEMO BINARY TREE
=========================================================
*/

void DemoBinaryTree() {

    cout << "\n===== BINARY TREE =====\n";

    BinaryTree<
        AscendingBinaryTreeTrait<int>
    > tree;

    tree.insert(8, 0);
    tree.insert(3, 0);
    tree.insert(10, 0);
    tree.insert(1, 0);
    tree.insert(6, 0);

    /*
    =====================================================
    operator <<
    =====================================================
    */

    cout << tree << endl;

    /*
    =====================================================
    FOREACH NATIVO (INORDER)
    =====================================================
    */

    cout << "\nINORDER:\n";

    for(auto x : tree) {
        cout << x << " ";
    }

    cout << endl;

    /*
    =====================================================
    PREORDER
    =====================================================
    */

    cout << "\nPREORDER:\n";

    for(auto it = tree.pbegin();
        it != tree.pend();
        ++it) {

        cout << *it << " ";
    }

    cout << endl;

    /*
    =====================================================
    POSTORDER
    =====================================================
    */

    cout << "\nPOSTORDER:\n";

    for(auto it = tree.postbegin();
        it != tree.postend();
        ++it) {

        cout << *it << " ";
    }

    cout << endl;

    /*
    =====================================================
    MEJORA LIBRE #1
    EXPORT DOT
    =====================================================
    */

    tree.exportDOT("bst.dot");

    cout << "\nArchivo bst.dot generado.\n";

    /*
    =====================================================
    MEJORA LIBRE #2
    METRICAS
    =====================================================
    */

    cout << "\nALTURA DEL ARBOL: "
         << tree.height()
         << endl;

    cout << "CANTIDAD DE HOJAS: "
         << tree.countLeaves()
         << endl;
}

/*
=========================================================
DEMO AVL TREE
=========================================================
*/

void DemoAVLTree() {

    cout << "\n===== AVL TREE =====\n";

    AVLTree<
        AscendingAVLTrait<int>
    > avl;

    avl.insert(30, 0);
    avl.insert(20, 0);
    avl.insert(10, 0);

    avl.insert(40, 0);
    avl.insert(50, 0);

    avl.insert(25, 0);

    /*
    =====================================================
    MOSTRAR AVL
    =====================================================
    */

    cout << avl << endl;

    /*
    =====================================================
    FOREACH NATIVO
    =====================================================
    */

    cout << "\nAVL INORDER:\n";

    for(auto x : avl) {
        cout << x << " ";
    }

    cout << endl;

    /*
    =====================================================
    EXPORT DOT
    =====================================================
    */

    avl.exportDOT("avl.dot");

    cout << "\nArchivo avl.dot generado.\n";

    /*
    =====================================================
    METRICAS AVL
    =====================================================
    */

    cout << "\nALTURA AVL: "
         << avl.height()
         << endl;

    cout << "HOJAS AVL: "
         << avl.countLeaves()
         << endl;
}