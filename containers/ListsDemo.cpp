#include <iostream>
#include <fstream>
#include <string>

#include "../types.h"
#include "linkedlist.h"
#include "heap.h"
#include <thread>
#include <sstream>
#include <utility>

using namespace std;

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);

    cout << list << endl;

    ofstream os(fileName);
    os << list << endl;

    ifstream is(fileName);
    is >> list;

    cout << list << endl;
}

void LinkedListDemo(){
    LinkedList<AscendingLinkedListTrait<T1>> list;
    DemoList(list, "AscLL.txt");

    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
}

void DoubleLinkedListDemo(){
    // DoubleLinkedList<T1, AscendingDLLTrait<T1>> list;
    // DemoList(list, "AscDLL.txt");

    // DoubleLinkedList<T1, DescendingDLLTrait<T1>> list2;
    // DemoList(list2, "DescDLL.txt");
}

void CircularLinkedListDemo(){

}

// =====================================================
// TEST CONCURRENCIA
// =====================================================

void TestConcurrencia(){

    cout << "\nTEST DE CONCURRENCIA" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> list;

    auto worker = [&list](int thread_id){

        for(int i = 0; i < 1000; i++){
            list.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;

    cout << "Tamano de la lista (Esperado 5000): "
         << list.size()
         << endl;

    if(list.size() == 5000){
        cout << "ESTADO: EXITO - El shared_mutex previno condiciones de carrera perfectamente." << endl;
    }
    else{
        cout << "ESTADO: FALLO - Hubo corrupcion de memoria." << endl;
    }
}

// =====================================================
// TEST OPERADORES
// =====================================================

void TestOperators(){

    cout << "\nTEST DE OPERADORES" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> list;

    cout << "Simulando lectura desde formato: [(10, 100), (20, 200), (30, 300)]" << endl;

    stringstream simulador_input(
        "[(10,100),(20,200),(30,300)]"
    );

    simulador_input >> list;

    cout << "Lista luego de la lectura (operator<<): "
         << list
         << endl;

    cout << "Accediendo al indice [0] (operator[]): Dato -> "
         << list[0]
         << endl;

    cout << "Accediendo al indice [2] (operator[]): Dato -> "
         << list[2]
         << endl;
}

// =====================================================
// TEST CONSTRUCTORES Y ASIGNACIONES
// =====================================================

void TestConstructores(){

    cout << "\nTEST DE CONSTRUCTORES" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> original;

    original.push_back(10, 100);
    original.push_back(20, 200);
    original.push_back(30, 300);

    cout << "Original: "
         << original
         << endl;

    LinkedList<AscendingLinkedListTrait<T1>>
        copia(original);

    cout << "Copy Constructor: "
         << copia
         << endl;

    LinkedList<AscendingLinkedListTrait<T1>>
        movida(std::move(copia));

    cout << "Move Constructor: "
         << movida
         << endl;

    LinkedList<AscendingLinkedListTrait<T1>>
        asignada;

    asignada = original;

    cout << "Copy Assignment: "
         << asignada
         << endl;

    LinkedList<AscendingLinkedListTrait<T1>>
        asignadaMove;

    asignadaMove = std::move(asignada);

    cout << "Move Assignment: "
         << asignadaMove
         << endl;
}

// =====================================================
// TEST POPS
// =====================================================

void TestPops(){

    cout << "\nTEST DE POPS" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> list;

    list.push_back(10, 100);
    list.push_back(20, 200);
    list.push_back(30, 300);

    cout << "Lista inicial: "
         << list
         << endl;

    auto front = list.pop_front();

    cout << "pop_front() -> ("
         << get<0>(front)
         << ","
         << get<1>(front)
         << ")"
         << endl;

    auto back = list.pop_back();

    cout << "pop_back() -> ("
         << get<0>(back)
         << ","
         << get<1>(back)
         << ")"
         << endl;

    cout << "Lista final: "
         << list
         << endl;
}

//Demo MinHeap
void DemoMinHeap(){

    cout << "\nTEST MIN HEAP" << endl;

    Heap< MinHeapTrait<int> > heap;

    heap.insert(50, 0);
    heap.insert(20, 0);
    heap.insert(80, 0);
    heap.insert(10, 0);
    heap.insert(30, 0);

    cout << "Heap: " << heap << endl;

    cout << "Peek: "
         << heap.peek().getData()
         << endl;

    heap.extract();

    cout << "Luego de extract(): "
         << heap
         << endl;
}

//Demo MAxHeap
void DemoMaxHeap(){

    cout << "\nTEST MAX HEAP" << endl;

    Heap< MaxHeapTrait<int> > heap;

    heap.insert(50, 0);
    heap.insert(20, 0);
    heap.insert(80, 0);
    heap.insert(10, 0);
    heap.insert(30, 0);

    cout << "Heap: " << heap << endl;

    cout << "Peek: "
         << heap.peek().getData()
         << endl;

    heap.extract();

    cout << "Luego de extract(): "
         << heap
         << endl;
}


// =====================================================
// DEMO GENERAL
// =====================================================

void ListsDemo(){

    TestConcurrencia();

    TestOperators();

    TestConstructores();

    TestPops();

    DemoMinHeap();
    DemoMaxHeap();

    cout << "\n=== FIN DE LAS PRUEBAS ===" << endl;
}