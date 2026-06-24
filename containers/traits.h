#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>
#include "BTreePage.h"

// =====================================================
// BaseTrait: para contenedores con nodos directos
// (LinkedList, AVL). _Node debe tener value_type.
// =====================================================

template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait
    : public BaseTrait<_Node, std::less<typename _Node::value_type>> {};

template <typename _Node>
struct DescendingTrait
    : public BaseTrait<_Node, std::greater<typename _Node::value_type>> {};

// =====================================================
// BaseHeapTrait: para Heap (compone Vector, no nodos)
// =====================================================

template <typename T, typename _Comp>
struct BaseHeapTrait {
    using value_type = T;
    using Comp       = _Comp;
};

// =====================================================
// BTree Traits:
// - value_type, ObjIDType, Comp vienen del Trait
// - Page = CBTreePage<Trait> se define DENTRO del Trait
//   (mismo patrón que Node = AVLTreeNode<...> en HashMapTrait)
//
// Uso:
//   BTree< AscendingBTreeTrait<T1> > bt;
//   BTree< DescendingBTreeTrait<T1> > bt;
// =====================================================

template<typename T, typename O = long>
struct AscendingBTreeTrait {
    using value_type = T;
    using ObjIDType  = O;
    using Comp       = std::less<T>;
    using Page       = CBTreePage< AscendingBTreeTrait<T,O> >;
};

template<typename T, typename O = long>
struct DescendingBTreeTrait {
    using value_type = T;
    using ObjIDType  = O;
    using Comp       = std::greater<T>;
    using Page       = CBTreePage< DescendingBTreeTrait<T,O> >;
};

#endif // __TRAITS_H__