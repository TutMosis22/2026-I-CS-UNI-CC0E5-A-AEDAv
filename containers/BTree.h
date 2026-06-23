#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"
#include "../types.h"

using namespace std;

#define DEFAULT_BTREE_ORDER 3

// =====================================================
// BTree: árbol B parametrizado por keyType y ObjIDType
//
// ForEach y FirstThat delegan en CBTreePage con
// templates variádicos — sin punteros a función C.
// =====================================================

template <typename keyType, typename ObjIDType = long>
class BTree
{
    using BTNode     = CBTreePage<keyType, ObjIDType>;
    using ObjectInfo = tagObjectInfo<keyType, ObjIDType>;

public:
    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true);
    ~BTree() {}

    bool      Insert(const keyType key, const ObjIDType ObjID);
    bool      Remove(const keyType key, const ObjIDType ObjID);
    ObjIDType Search(const keyType key);

    long size()     { return m_NumKeys; }
    long height()   { return m_Height;  }
    long GetOrder() { return m_Order;   }

    void Print(ostream& os) { m_Root.Print(os); }

    // =====================================================
    // ForEach variadic: pasa func y args directamente a
    // CBTreePage::ForEach sin intermediarios ni casting
    // =====================================================
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args)
    {
        m_Root.ForEach(func, (size_t)0, forward<Args>(args)...);
    }

    // =====================================================
    // FirstThat variadic: retorna ObjectInfo* del primer
    // elemento que cumple el predicado, o nullptr
    // =====================================================
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args)
    {
        return m_Root.FirstThat(func, (size_t)0, forward<Args>(args)...);
    }

protected:
    BTNode m_Root;
    long   m_Height;
    size_t m_Order;
    long   m_NumKeys;
    bool   m_Unique;
};

template <typename keyType, typename ObjIDType>
BTree<keyType, ObjIDType>::BTree(size_t order, bool unique)
    : m_Unique(unique),
      m_Order(order),
      m_Root(2 * order + 1, unique),
      m_NumKeys(0),
      m_Height(1)
{
    m_Root.SetMaxKeysForChilds(order);
}

template <typename keyType, typename ObjIDType>
bool BTree<keyType, ObjIDType>::Insert(const keyType key, const ObjIDType ObjID)
{
    bt_ErrorCode error = m_Root.Insert(key, ObjID);
    if (error == bt_duplicate) return false;
    m_NumKeys++;
    if (error == bt_overflow) {
        m_Root.SplitRoot();
        m_Height++;
    }
    return true;
}

template <typename keyType, typename ObjIDType>
bool BTree<keyType, ObjIDType>::Remove(const keyType key, const ObjIDType ObjID)
{
    bt_ErrorCode error = m_Root.Remove(key, ObjID);
    if (error == bt_duplicate || error == bt_nofound) return false;
    m_NumKeys--;
    if (error == bt_rootmerged) m_Height--;
    return true;
}

template <typename keyType, typename ObjIDType>
ObjIDType BTree<keyType, ObjIDType>::Search(const keyType key)
{
    ObjIDType ObjID = -1;
    m_Root.Search(key, ObjID);
    return ObjID;
}

#endif // BTREE_H