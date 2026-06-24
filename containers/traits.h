#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>

// =====================================================
// BaseTrait: base para contenedores con nodos directos
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
// Solo expone value_type y Comp.
// =====================================================

template <typename T, typename _Comp>
struct BaseHeapTrait {
    using value_type = T;
    using Comp       = _Comp;
};

// =====================================================
// BTree Traits: mismo patrón que MinHeapTrait/MaxHeapTrait.
// BTree compone páginas internas, no nodos directos,
// por eso el Trait solo expone value_type, ObjIDType y Comp.
//
// ObjIDType = long por defecto (ID del objeto almacenado).
// =====================================================

template<typename T, typename O = long>
struct AscendingBTreeTrait {
    using value_type = T;
    using ObjIDType  = O;
    using Comp       = std::less<T>;
};

template<typename T, typename O = long>
struct DescendingBTreeTrait {
    using value_type = T;
    using ObjIDType  = O;
    using Comp       = std::greater<T>;
};

#endif // __TRAITS_H__