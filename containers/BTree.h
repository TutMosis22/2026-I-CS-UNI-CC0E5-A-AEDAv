#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "traits.h"
#include "../types.h"

using namespace std;

#define DEFAULT_BTREE_ORDER 3

// =====================================================
// BTree: recibe Trait completo (mismo patrón que
// LinkedList<Trait>, Heap<Trait>, HashMap<Trait>).
//
// Del Trait extrae:
//   value_type  — tipo de las claves
//   ObjIDType   — tipo del identificador almacenado
//   Comp        — comparador (less / greater)
//   Page        — CBTreePage<Trait> definido en el Trait
//
// Uso:
//   BTree< AscendingBTreeTrait<T1> > bt;
//   BTree< DescendingBTreeTrait<T1> > bt;
// =====================================================

template <typename Trait>
class BTree
{
public:
    using value_type = typename Trait::value_type;
    using ObjIDType  = typename Trait::ObjIDType;
    using Comp       = typename Trait::Comp;
    using MySelf     = BTree<Trait>;
    using BTNode     = typename Trait::Page;
    using ObjectInfo = tagObjectInfo<value_type, ObjIDType>;

public:
    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true);
    ~BTree() {}

    bool      Insert(const value_type key, const ObjIDType ObjID);
    bool      Remove(const value_type key, const ObjIDType ObjID);
    ObjIDType Search(const value_type key);

    long   size()     { return m_NumKeys; }
    long   height()   { return m_Height;  }
    size_t GetOrder() { return m_Order;   }

    // ForEach variadic
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args)
    {
        m_Root.ForEach(func, (size_t)0, forward<Args>(args)...);
    }

    // FirstThat variadic (forward)
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args)
    {
        return m_Root.FirstThat(func, (size_t)0, forward<Args>(args)...);
    }

    // ForEach backward (inorder descendente)
    template <typename Func, typename... Args>
    void ForEachReverse(Func func, Args&&... args)
    {
        m_Root.ForEachReverse(func, (size_t)0, forward<Args>(args)...);
    }

    // FirstThat backward
    template <typename Func, typename... Args>
    ObjectInfo* FirstThatReverse(Func func, Args&&... args)
    {
        return m_Root.FirstThatReverse(func, (size_t)0, forward<Args>(args)...);
    }

    // operator<<: imprime el árbol usando ForEach (ya no usa Print helper)
    friend ostream& operator<<(ostream& os, BTree& bt)
    {
        bt.m_Root.ForEach(
            [](ObjectInfo& info, size_t level, ostream& out) {
                for (size_t i = 0; i < level; i++) out << "\t";
                out << info.key << "->" << info.ObjID << "\n";
            },
            (size_t)0, os);
        return os;
    }

    // operator>>: lee pares "key:id" separados por espacios
    friend istream& operator>>(istream& is, BTree& bt)
    {
        value_type key;
        ObjIDType  id;
        char       colon;
        while (is >> key >> colon >> id)
            bt.Insert(key, id);
        return is;
    }

protected:
    BTNode m_Root;
    long   m_Height;
    size_t m_Order;
    long   m_NumKeys;
    bool   m_Unique;
};

template <typename Trait>
BTree<Trait>::BTree(size_t order, bool unique)
    : m_Unique(unique),
      m_Order(order),
      m_Root(2 * order + 1, unique),
      m_NumKeys(0),
      m_Height(1)
{
    m_Root.SetMaxKeysForChilds(order);
}

template <typename Trait>
bool BTree<Trait>::Insert(const value_type key, const ObjIDType ObjID)
{
    bt_ErrorCode error = m_Root.Insert(key, ObjID);
    if (error == bt_duplicate) return false;
    m_NumKeys++;
    if (error == bt_overflow) { m_Root.SplitRoot(); m_Height++; }
    return true;
}

template <typename Trait>
bool BTree<Trait>::Remove(const value_type key, const ObjIDType ObjID)
{
    bt_ErrorCode error = m_Root.Remove(key, ObjID);
    if (error == bt_duplicate || error == bt_nofound) return false;
    m_NumKeys--;
    if (error == bt_rootmerged) m_Height--;
    return true;
}

template <typename Trait>
typename BTree<Trait>::ObjIDType
BTree<Trait>::Search(const value_type key)
{
    ObjIDType ObjID = -1;
    m_Root.Search(key, ObjID);
    return ObjID;
}

#endif // BTREE_H