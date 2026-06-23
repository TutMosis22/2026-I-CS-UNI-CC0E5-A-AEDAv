#include <iostream>
#include <sstream>
#include <string>
#include "BTree.h"
#include "../types.h"

using namespace std;

// =====================================================
// DemoBTree: demuestra BTree<T1> con ForEach y FirstThat
// variádicos — sin punteros a función estilo C.
// Sigue el mismo patrón que DemoMinHeap/DemoMaxHeap.
// =====================================================

void DemoBTree()
{
    cout << "\nTEST BTREE" << endl;

    BTree<T1> bt(3);

    // Inserción
    const T1 keys[] = {50, 30, 70, 10, 40, 60, 80, 20, 90};
    const size_t n  = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < n; i++)
        bt.Insert(keys[i], static_cast<long>(i));

    cout << "Arbol luego de inserciones:" << endl;
    bt.Print(cout);

    cout << "Size   : " << bt.size()     << endl;
    cout << "Height : " << bt.height()   << endl;
    cout << "Order  : " << bt.GetOrder() << endl;

    // ForEach variadic con lambda y argumento extra (ostream)
    cout << "\nForEach (inorder con nivel):" << endl;
    bt.ForEach(
        [](tagObjectInfo<T1, long>& info, size_t level, ostream& os)
        {
            for (size_t i = 0; i < level; i++) os << "  ";
            os << "key=" << info.key
               << " id=" << info.ObjID << "\n";
        },
        cout
    );

    // FirstThat variadic con lambda y argumento extra (umbral)
    cout << "\nFirstThat (primer elemento con key > 45):" << endl;
    T1 umbral = 45;
    auto* found = bt.FirstThat(
        [](tagObjectInfo<T1, long>& info, size_t /*level*/, T1 threshold)
        {
            return info.key > threshold;
        },
        umbral
    );

    if (found)
        cout << "Encontrado: key=" << found->key
             << " id=" << found->ObjID << endl;
    else
        cout << "No encontrado." << endl;

    // Search
    cout << "\nSearch(40): ObjID = " << bt.Search(40) << endl;
    cout << "Search(99): ObjID = " << bt.Search(99) << " (esperado -1)" << endl;

    // Remove
    bt.Remove(30, 0);
    cout << "\nLuego de Remove(30):" << endl;
    bt.Print(cout);
}