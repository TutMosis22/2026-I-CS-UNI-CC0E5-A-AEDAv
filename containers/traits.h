#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para less y greater

// =====================================================
// BaseTrait para LinkedList: _Node es un nodo con ::value_type
// =====================================================
template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, less<typename _Node::value_type>>{
};

template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, greater<typename _Node::value_type>>{
};

// =====================================================
// BaseHeapTrait para Heap: T es el tipo de valor directamente
// =====================================================
template <typename T, typename _Comp>
struct BaseHeapTrait{
    using value_type = T;
    using Comp       = _Comp;
};

#endif // __TRAITS_H__