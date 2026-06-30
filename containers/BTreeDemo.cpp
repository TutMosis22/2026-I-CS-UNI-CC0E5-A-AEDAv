#include <iostream>
#include <sstream>
#include "BTree.h"
#include "../types.h"

using namespace std;

// =====================================================
// DemoBTree: mismo patrón que DemoMinHeap/DemoMaxHeap.
// El Trait se inyecta desde afuera — BTree es genérico.
// =====================================================

void DemoBTree()
{
    cout << "\nTEST BTREE (Ascendente)" << endl;

    BTree< AscendingBTreeTrait<T1> > bt(3);

    const T1 keys[] = {50, 30, 70, 10, 40, 60, 80, 20, 90};
    const size_t n  = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < n; i++)
        bt.Insert(keys[i], static_cast<long>(i));

    cout << "Arbol luego de inserciones:" << endl;
    cout << bt;
    cout << "Size   : " << bt.size()     << endl;
    cout << "Height : " << bt.height()   << endl;
    cout << "Order  : " << bt.GetOrder() << endl;

    // ForEach variadic con lambda y argumento extra
    cout << "\nForEach (inorder con nivel):" << endl;
    bt.ForEach(
        [](tagObjectInfo<T1,long>& info, size_t level, ostream& os) {
            for (size_t i = 0; i < level; i++) os << "  ";
            os << "key=" << info.key << " id=" << info.ObjID << "\n";
        },
        cout
    );

    // FirstThat variadic con lambda y argumento extra
    cout << "\nFirstThat (primer elemento con key > 45):" << endl;
    T1 umbral = 45;
    auto* found = bt.FirstThat(
        [](tagObjectInfo<T1,long>& info, size_t, T1 threshold) {
            return info.key > threshold;
        },
        umbral
    );
    if (found)
        cout << "Encontrado: key=" << found->key << " id=" << found->ObjID << endl;
    else
        cout << "No encontrado." << endl;

    cout << "\nSearch(40): ObjID = " << bt.Search(40) << endl;
    cout << "Search(99): ObjID = " << bt.Search(99) << " (esperado -1)" << endl;

    bt.Remove(30, 0);
    cout << "\nLuego de Remove(30):" << endl;
    cout << bt;

    // Mismo contenedor, Trait descendente
    cout << "\nTEST BTREE (Descendente)" << endl;
    BTree< DescendingBTreeTrait<T1> > bt2(3);
    for (size_t i = 0; i < n; i++)
        bt2.Insert(keys[i], static_cast<long>(i));
    cout << "Arbol descendente (ForEachReverse):" << endl;
    bt2.ForEachReverse(
        [](tagObjectInfo<T1,long>& info, size_t level, ostream& os) {
            for (size_t i = 0; i < level; i++) os << "  ";
            os << "key=" << info.key << " id=" << info.ObjID << "\n";
        },
        cout
    );

    // operator>> — leer árbol desde stream
    cout << "\nBTree leido con operator>>:" << endl;
    BTree< AscendingBTreeTrait<T1> > bt3(3);
    {
        stringstream ss("1:100 2:200 3:300 4:400 5:500");
        ss >> bt3;
    }
    cout << bt3;
}