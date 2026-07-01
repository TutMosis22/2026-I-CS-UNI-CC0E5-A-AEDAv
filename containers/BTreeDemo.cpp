#include <iostream>
#include <sstream>
#include "BTree.h"
#include "../types.h"

using namespace std;

void DemoBTree()
{
    cout << "\nTEST BTREE (Ascendente)" << endl;

    BTree< AscendingBTreeTrait<T1> > bt(3);

    const T1 keys[] = {50, 30, 70, 10, 40, 60, 80, 20, 90};
    const size_t n  = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < n; i++)
        bt.Insert(keys[i], static_cast<long>(i));

    // operator<< — usa ForEach internamente
    cout << "Arbol luego de inserciones:" << endl;
    cout << bt;

    cout << "Size   : " << bt.size()     << endl;
    cout << "Height : " << bt.height()   << endl;
    cout << "Order  : " << bt.GetOrder() << endl;

    // ForEach forward con lambda
    cout << "\nForEach forward (inorder ascendente):" << endl;
    bt.ForEach(
        [](tagObjectInfo<T1,long>& info, ostream& os) {
            os << "  key=" << info.key << " id=" << info.ObjID << "\n";
        },
        cout
    );

    // ForEach backward con lambda
    cout << "\nForEachReverse (inorder descendente):" << endl;
    bt.ForEachReverse(
        [](tagObjectInfo<T1,long>& info, ostream& os) {
            os << "  key=" << info.key << " id=" << info.ObjID << "\n";
        },
        cout
    );

    // FirstThat forward
    cout << "\nFirstThat (primer elemento con key > 45):" << endl;
    T1 umbral = 45;
    auto* found = bt.FirstThat(
        [](tagObjectInfo<T1,long>& info, T1 threshold) {
            return info.key > threshold;
        },
        umbral
    );
    if (found)
        cout << "Encontrado: key=" << found->key << " id=" << found->ObjID << endl;
    else
        cout << "No encontrado." << endl;

    // FirstThatReverse
    cout << "\nFirstThatReverse (ultimo elemento con key < 45):" << endl;
    auto* found2 = bt.FirstThatReverse(
        [](tagObjectInfo<T1,long>& info, T1 threshold) {
            return info.key < threshold;
        },
        umbral
    );
    if (found2)
        cout << "Encontrado: key=" << found2->key << " id=" << found2->ObjID << endl;
    else
        cout << "No encontrado." << endl;

    cout << "\nSearch(40): ObjID = " << bt.Search(40) << endl;
    cout << "Search(99): ObjID = " << bt.Search(99) << " (esperado -1)" << endl;

    bt.Remove(30, 0);
    cout << "\nLuego de Remove(30):" << endl;
    cout << bt;

    // operator>> — leer árbol desde stream
    cout << "\nBTree leido con operator>>:" << endl;
    BTree< AscendingBTreeTrait<T1> > bt2(3);
    {
        stringstream ss("1:100 2:200 3:300 4:400 5:500");
        ss >> bt2;
    }
    cout << bt2;

    // Mismo contenedor, Trait descendente
    cout << "\nTEST BTREE (Descendente)" << endl;
    BTree< DescendingBTreeTrait<T1> > bt3(3);
    for (size_t i = 0; i < n; i++)
        bt3.Insert(keys[i], static_cast<long>(i));
    cout << bt3;
}