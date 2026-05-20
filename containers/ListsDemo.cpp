#include <iostream>
#include <fstream>
#include <thread>

#include "../types.h"

#include "BinaryTree.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"

using namespace std;

///////////////////////////////////////////////////////////////
// FUNCIONES AUXILIARES
///////////////////////////////////////////////////////////////

void AddOne(int& x){
    x++;
}

///////////////////////////////////////////////////////////////
// DEMO AVL TREE
///////////////////////////////////////////////////////////////

void DemoAVLTree(){

    cout << "\n======================================" << endl;
    cout << "        DEMO AVL TREE" << endl;
    cout << "======================================" << endl;

    AVLTree<AscendingTrait<AVLNode<T1>>> tree;

    ///////////////////////////////////////////////////////////
    // INSERT
    ///////////////////////////////////////////////////////////

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    ///////////////////////////////////////////////////////////
    // TOSTRING
    ///////////////////////////////////////////////////////////

    cout << "\nToString():" << endl;
    cout << tree.toString() << endl;

    ///////////////////////////////////////////////////////////
    // INORDER
    ///////////////////////////////////////////////////////////

    cout << "\nInorder Forward Iterator:" << endl;

    for(auto x : tree){
        cout << x << " ";
    }

    cout << endl;

    ///////////////////////////////////////////////////////////
    // PREORDER
    ///////////////////////////////////////////////////////////

    cout << "\nPreorder Traversal:" << endl;

    auto pre = tree.preorder();

    for(auto x : pre){
        cout << x << " ";
    }

    cout << endl;

    ///////////////////////////////////////////////////////////
    // POSTORDER
    ///////////////////////////////////////////////////////////

    cout << "\nPostorder Traversal:" << endl;

    auto post = tree.postorder();

    for(auto x : post){
        cout << x << " ";
    }

    cout << endl;

    ///////////////////////////////////////////////////////////
    // PERSISTENCIA
    ///////////////////////////////////////////////////////////

    ofstream ofs("avl.txt");

    ofs << tree;

    ofs.close();

    cout << "\nArchivo avl.txt generado correctamente." << endl;

    ///////////////////////////////////////////////////////////
    // OPERATOR >>
    ///////////////////////////////////////////////////////////

    AVLTree<AscendingTrait<AVLNode<T1>>> tree2;

    ifstream ifs("avl.txt");

    ifs >> tree2;

    ifs.close();

    cout << "\nTree cargado desde archivo:" << endl;

    cout << tree2 << endl;

    ///////////////////////////////////////////////////////////
    // CONCURRENCIA
    ///////////////////////////////////////////////////////////

    cout << "\nTest de concurrencia:" << endl;

    AVLTree<AscendingTrait<AVLNode<T1>>> concurrentTree;

    auto worker = [&concurrentTree](int start){

        for(int i=0;i<1000;i++){
            concurrentTree.insert(start + i, i);
        }

    };

    thread t1(worker, 0);
    thread t2(worker, 10000);
    thread t3(worker, 20000);

    t1.join();
    t2.join();
    t3.join();

    cout << "Inserciones concurrentes completadas." << endl;

    cout << "Cantidad de elementos: "
         << concurrentTree.size()
         << endl;
}

///////////////////////////////////////////////////////////////
// DEMO DOUBLE LINKED LIST
///////////////////////////////////////////////////////////////

void DemoDoubleLinkedList(){

    cout << "\n======================================" << endl;
    cout << "     DEMO DOUBLE LINKED LIST" << endl;
    cout << "======================================" << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> list;

    list.push_back(10,1);
    list.push_back(20,2);
    list.push_back(30,3);

    ///////////////////////////////////////////////////////////
    // FOREACH NATIVO
    ///////////////////////////////////////////////////////////

    cout << "\nForeach nativo:" << endl;

    for(auto x : list){
        cout << x << " ";
    }

    cout << endl;

    ///////////////////////////////////////////////////////////
    // ITERADOR BACKWARD
    ///////////////////////////////////////////////////////////

    cout << "\nBackward Iterator:" << endl;

    for(auto it = list.rbegin(); it != list.rend(); ++it){
        cout << *it << " ";
    }

    cout << endl;

    ///////////////////////////////////////////////////////////
    // TOSTRING
    ///////////////////////////////////////////////////////////

    cout << "\nToString:" << endl;

    cout << list.toString() << endl;
}

///////////////////////////////////////////////////////////////
// MAIN DEMO
///////////////////////////////////////////////////////////////

void ListsDemo(){

    DemoAVLTree();

    DemoDoubleLinkedList();

    cout << "\n======================================" << endl;
    cout << "     FIN DE TODAS LAS PRUEBAS" << endl;
    cout << "======================================" << endl;
}