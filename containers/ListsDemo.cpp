#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <sstream>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

// =========================
// DEMO GENERICO
// =========================
template <typename Container>
void DemoList(Container& list, string fileName){
    cout << "\n=== DEMO BASICO ===\n";

    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);

    cout << "Lista luego de inserts: " << list << endl;

    // Escritura
    ofstream os(fileName);
    os << list << endl;

    // Lectura
    ifstream is(fileName);
    is >> list;

    cout << "Lista luego de lectura: " << list << endl;

    // operator[]
    cout << "Elemento [0]: " << list[0] << endl;

    // pop
    auto front = list.pop_front();
    cout << "pop_front: (" << get<0>(front) << "," << get<1>(front) << ")" << endl;

    auto back = list.pop_back();
    cout << "pop_back: (" << get<0>(back) << "," << get<1>(back) << ")" << endl;

    cout << "Lista final: " << list << endl;
}

// =========================
// LINKED LIST
// =========================
void LinkedListDemo(){
    cout << "\n============================";
    cout << "\nLINKED LIST DEMO";
    cout << "\n============================\n";

    LinkedList<AscendingLinkedListTrait<T1>> list;
    DemoList(list, "AscLL.txt");

    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
}

// =========================
// DOUBLE LINKED LIST
// =========================
void DoubleLinkedListDemo(){
    cout << "\n============================";
    cout << "\nDOUBLE LINKED LIST DEMO";
    cout << "\n============================\n";

    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    DemoList(list, "AscDLL.txt");

    DoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DescDLL.txt");

    // recorrido backward (extra importante)
    cout << "\nRecorrido manual backward:\n";
    auto it = list.begin();
    while(it != list.end()){
        cout << *it << " ";
        ++it;
    }
    cout << endl;
}

// =========================
// CIRCULAR LINKED LIST
// =========================
void CircularLinkedListDemo(){
    cout << "\n============================";
    cout << "\nCIRCULAR LINKED LIST DEMO";
    cout << "\n============================\n";

    CircularLinkedList<AscendingLinkedListTrait<T1>> list;

    list.insert(10, 1);
    list.insert(20, 2);
    list.insert(30, 3);

    cout << "Recorrido circular (2 vueltas):\n";
    list.circularForEach([](auto& x){
        cout << x << " ";
    }, 2);

    cout << endl;
}

// =========================
// CIRCULAR DOUBLE LINKED LIST
// =========================
void CircularDoubleLinkedListDemo(){
    cout << "\n============================";
    cout << "\nCIRCULAR DOUBLE LINKED LIST DEMO";
    cout << "\n============================\n";

    CircularDoubleLinkedList<AscendingDLLTrait<T1>> list;

    list.insert(5, 1);
    list.insert(15, 2);
    list.insert(25, 3);

    cout << "Forward (2 vueltas):\n";
    list.circularForEach([](auto& x){
        cout << x << " ";
    }, 2, 1);

    cout << "\nBackward (2 vueltas):\n";
    list.circularForEach([](auto& x){
        cout << x << " ";
    }, 2, -1);

    cout << endl;
}

// =========================
// TEST CONCURRENCIA
// =========================
void TestConcurrencia() {
    cout << "\n============================";
    cout << "\nTEST CONCURRENCIA";
    cout << "\n============================\n";

    LinkedList<AscendingLinkedListTrait<T1>> list;

    auto worker = [&list](int thread_id) {
        for(int i = 0; i < 1000; i++) {
            list.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Tamano esperado: 5000\n";
    cout << "Tamano real: " << list.size() << endl;
}

// =========================
// TEST OPERADORES
// =========================
void TestOperators() {
    cout << "\n============================";
    cout << "\nTEST OPERADORES";
    cout << "\n============================\n";

    LinkedList<AscendingLinkedListTrait<T1>> list;

    stringstream ss("[(10,100),(20,200),(30,300)]");

    ss >> list;

    cout << "Lista leida: " << list << endl;

    cout << "Index [1]: " << list[1] << endl;
}

// =========================
// MAIN DEMO
// =========================
void ListsDemo(){
    LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();

    TestConcurrencia();
    TestOperators();

    cout << "\n=== FIN DE LAS PRUEBAS ===\n";
}