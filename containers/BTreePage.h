#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <cassert>
#include "../types.h"

template <typename keyType, typename ObjIDType>
class BTree;

using namespace std;

enum bt_ErrorCode {
    bt_ok, bt_overflow, bt_underflow,
    bt_duplicate, bt_nofound, bt_rootmerged
};

// =====================================================
// tagObjectInfo: par (key, ObjID) almacenado en cada
// página del B-Tree. ObjIDType parametriza el tipo de
// referencia (long por defecto en BTree.h).
// =====================================================

template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
    keyType   key;
    ObjIDType ObjID;
    long      UseCounter;

    tagObjectInfo(const keyType& _key, ObjIDType _ObjID)
        : key(_key), ObjID(_ObjID), UseCounter(0) {}

    tagObjectInfo() {}

    operator keyType() { return key; }

    long GetUseCounter() { return UseCounter; }
};

// =====================================================
// Helpers de bajo nivel: binary_search, insert_at, remove
// Operan sobre vectores genéricos — sin tipos primitivos
// hardcodeados: usan size_t para índices.
// =====================================================

template <typename Container, typename ObjType>
size_t binary_search(Container& container, size_t first, size_t last, ObjType& object)
{
    if (first >= last) return first;
    while (first < last) {
        size_t mid = (first + last) / 2;
        if (object == (ObjType)container[mid]) return mid;
        if (object >  (ObjType)container[mid]) first = mid + 1;
        else                                   last  = mid;
    }
    if (object <= (ObjType)container[first]) return first;
    return last;
}

template <typename Container, typename ObjType>
void insert_at(Container& container, const ObjType& object, size_t pos)
{
    size_t size = container.size();
    for (size_t i = size - 1; i > pos; i--)
        container[i] = container[i - 1];
    container[pos] = object;
}

template <typename Container>
void remove_at(Container& container, size_t pos)
{
    size_t size = container.size();
    for (size_t i = pos + 1; i < size; i++)
        container[i - 1] = container[i];
}

// =====================================================
// CBTreePage: página del B-Tree en memoria
//
// ForEach y FirstThat usan templates variádicos en vez
// de punteros a función estilo C (lpfnForEach2/3).
// Esto elimina la limitación de exactamente 2 ó 3
// parámetros extra y permite lambdas y functors.
// =====================================================

template <typename keyType, typename ObjIDType>
class CBTreePage
{
    friend class BTree<keyType, ObjIDType>;

    using BTPage     = CBTreePage<keyType, ObjIDType>;
    using ObjectInfo = tagObjectInfo<keyType, ObjIDType>;

public:
    CBTreePage(size_t maxKeys, bool unique = true);
    virtual ~CBTreePage();

    bt_ErrorCode Insert(const keyType& key, const ObjIDType ObjID);
    bt_ErrorCode Remove(const keyType& key, const ObjIDType ObjID);
    bool         Search(const keyType& key, ObjIDType& ObjID);
    void         Print(ostream& os);

    // =====================================================
    // ForEach variadic: recorre todos los elementos inorder
    // func recibe (ObjectInfo&, size_t level, Args...)
    // =====================================================
    template <typename Func, typename... Args>
    void ForEach(Func func, size_t level, Args&&... args)
    {
        for (size_t i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i])
                m_SubPages[i]->ForEach(func, level + 1,
                                       forward<Args>(args)...);
            func(m_Keys[i], level, forward<Args>(args)...);
        }
        if (m_SubPages[m_KeyCount])
            m_SubPages[m_KeyCount]->ForEach(func, level + 1,
                                            forward<Args>(args)...);
    }

    // =====================================================
    // FirstThat variadic: retorna puntero al primer elemento
    // para el cual func retorna true, o nullptr si no hay.
    // func recibe (ObjectInfo&, size_t level, Args...)
    // =====================================================
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, size_t level, Args&&... args)
    {
        ObjectInfo* pTmp = nullptr;
        for (size_t i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i]) {
                pTmp = m_SubPages[i]->FirstThat(func, level + 1,
                                                forward<Args>(args)...);
                if (pTmp) return pTmp;
            }
            if (func(m_Keys[i], level, forward<Args>(args)...))
                return &m_Keys[i];
        }
        if (m_SubPages[m_KeyCount]) {
            pTmp = m_SubPages[m_KeyCount]->FirstThat(func, level + 1,
                                                     forward<Args>(args)...);
            if (pTmp) return pTmp;
        }
        return nullptr;
    }

protected:
    // Contadores e índices: size_t en vez de int
    size_t m_MinKeys;
    size_t m_MaxKeys;
    size_t m_MaxKeysForChilds;
    bool   m_Unique;
    bool   m_isRoot;

    vector<ObjectInfo> m_Keys;
    vector<BTPage*>    m_SubPages;
    size_t             m_KeyCount;

    void Create();
    void Reset();
    void Destroy() { Reset(); delete this; }
    void clear();

    bool Redistribute1(size_t& pos);
    bool Redistribute2(size_t  pos);
    void RedistributeR2L(size_t pos);
    void RedistributeL2R(size_t pos);

    bool TreatUnderflow(size_t& pos) {
        return Redistribute1(pos) || Redistribute2(pos);
    }

    bt_ErrorCode Merge    (size_t pos);
    bt_ErrorCode MergeRoot();
    void         SplitChild(size_t pos);

    ObjectInfo& GetFirstObjectInfo();

    bool   Overflow()        { return m_KeyCount > m_MaxKeys; }
    bool   Underflow()       { return m_KeyCount < MinNumberOfKeys(); }
    bool   IsFull()          { return m_KeyCount >= m_MaxKeys; }
    size_t MinNumberOfKeys() { return 2 * m_MaxKeys / 3; }
    size_t GetFreeCells()    { return m_MaxKeys - m_KeyCount; }
    size_t& NumberOfKeys()   { return m_KeyCount; }
    size_t  GetNumberOfKeys(){ return m_KeyCount; }
    bool   IsRoot()          { return m_MaxKeysForChilds != m_MaxKeys; }

    void SetMaxKeysForChilds(size_t orderforchilds) {
        m_MaxKeysForChilds = orderforchilds;
    }

    size_t GetFreeCellsOnLeft (size_t pos);
    size_t GetFreeCellsOnRight(size_t pos);

private:
    bool SplitRoot();
    void SplitPageInto3(vector<ObjectInfo>& tmpKeys,
                        vector<BTPage*>&    SubPages,
                        BTPage*&            pChild1,
                        BTPage*&            pChild2,
                        BTPage*&            pChild3,
                        ObjectInfo&         oi1,
                        ObjectInfo&         oi2);
    void MovePage(BTPage* pChildPage,
                  vector<ObjectInfo>& tmpKeys,
                  vector<BTPage*>&    tmpSubPages);
};

// =====================================================
// IMPLEMENTACIONES
// =====================================================

template <typename keyType, typename ObjIDType>
CBTreePage<keyType, ObjIDType>::CBTreePage(size_t maxKeys, bool unique)
    : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
    Create();
    SetMaxKeysForChilds(m_MaxKeys);
}

template <typename keyType, typename ObjIDType>
CBTreePage<keyType, ObjIDType>::~CBTreePage()
{
    Reset();
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::Create()
{
    Reset();
    m_Keys.resize(m_MaxKeys + 1);
    m_SubPages.resize(m_MaxKeys + 2, nullptr);
    m_KeyCount = 0;
    m_MinKeys  = 2 * m_MaxKeys / 3;
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::Reset()
{
    // Borra subpáginas: hay m_KeyCount+1 posibles hijos
    // Solo borra si el vector fue inicializado y el puntero no es nulo
    size_t n = m_SubPages.size();
    for (size_t i = 0; i < n; i++)
        if (m_SubPages[i]) {
            delete m_SubPages[i];
            m_SubPages[i] = nullptr;
        }
    clear();
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::clear()
{
    m_KeyCount = 0;
}

template <typename keyType, typename ObjIDType>
bt_ErrorCode CBTreePage<keyType, ObjIDType>::Insert(
    const keyType& key, const ObjIDType ObjID)
{
    size_t pos = binary_search(m_Keys, (size_t)0, m_KeyCount, key);
    bt_ErrorCode error = bt_ok;

    if (pos < m_KeyCount && (keyType)m_Keys[pos] == key && m_Unique)
        return bt_duplicate;

    if (!m_SubPages[pos]) {
        ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
        NumberOfKeys()++;
        if (Overflow()) return bt_overflow;
        return bt_ok;
    }
    else {
        error = m_SubPages[pos]->Insert(key, ObjID);
        if (error == bt_overflow) {
            if (!Redistribute1(pos))
                SplitChild(pos);
            if (Overflow()) return bt_overflow;
            return bt_ok;
        }
    }

    if (Overflow()) return bt_overflow;
    return bt_ok;
}

template <typename keyType, typename ObjIDType>
bool CBTreePage<keyType, ObjIDType>::Redistribute1(size_t& pos)
{
    if (m_SubPages[pos]->Underflow()) {
        size_t nkol = 0, nkor = 0;
        if (pos > 0)              nkol = m_SubPages[pos-1]->NumberOfKeys();
        if (pos < NumberOfKeys()) nkor = m_SubPages[pos+1]->NumberOfKeys();

        if (nkol > nkor) {
            if (m_SubPages[pos-1]->NumberOfKeys() >
                m_SubPages[pos-1]->MinNumberOfKeys())
                RedistributeL2R(pos - 1);
            else {
                if (pos == NumberOfKeys()) { --pos; return false; }
                else return false;
            }
        }
        else {
            if (m_SubPages[pos+1]->NumberOfKeys() >
                m_SubPages[pos+1]->MinNumberOfKeys())
                RedistributeR2L(pos + 1);
            else {
                if (pos == 0) { ++pos; return false; }
                else return false;
            }
        }
    }
    else {
        size_t fcol = GetFreeCellsOnLeft(pos);
        size_t fcor = GetFreeCellsOnRight(pos);

        if (!fcol && !fcor && m_SubPages[pos]->IsFull())
            return false;
        if (fcol > fcor) RedistributeR2L(pos);
        else             RedistributeL2R(pos);
    }
    return true;
}

template <typename keyType, typename ObjIDType>
bool CBTreePage<keyType, ObjIDType>::Redistribute2(size_t pos)
{
    assert(pos > 0 && pos < NumberOfKeys());
    assert(m_SubPages[pos-1] && m_SubPages[pos] && m_SubPages[pos+1]);
    assert(m_SubPages[pos-1]->Underflow() ||
           m_SubPages[pos  ]->Underflow() ||
           m_SubPages[pos+1]->Underflow());

    if (m_SubPages[pos-1]->Underflow()) {
        RedistributeR2L(pos + 1);
        RedistributeR2L(pos);
        if (m_SubPages[pos-1]->Underflow()) return false;
    }
    else if (m_SubPages[pos+1]->Underflow()) {
        RedistributeL2R(pos - 1);
        RedistributeL2R(pos);
        if (m_SubPages[pos+1]->Underflow()) return false;
    }
    else {
        RedistributeL2R(pos - 1);
        RedistributeR2L(pos + 1);
        if (m_SubPages[pos]->Underflow()) return false;
    }
    return true;
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::RedistributeR2L(size_t pos)
{
    BTPage* pSource = m_SubPages[pos];
    BTPage* pTarget = m_SubPages[pos - 1];

    while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
           pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
    {
        ::insert_at(pTarget->m_Keys,    m_Keys[pos-1], pTarget->NumberOfKeys()++);
        ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

        m_Keys[pos-1] = pSource->m_Keys[0];
        ::remove_at(pSource->m_Keys,    0);
        ::remove_at(pSource->m_SubPages, 0);
        pSource->NumberOfKeys()--;
    }
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::RedistributeL2R(size_t pos)
{
    BTPage* pSource = m_SubPages[pos];
    BTPage* pTarget = m_SubPages[pos + 1];

    while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
           pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
    {
        ::insert_at(pTarget->m_Keys,    m_Keys[pos], (size_t)0);
        ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], (size_t)0);
        pTarget->NumberOfKeys()++;

        m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys() - 1];
        pSource->NumberOfKeys()--;
    }
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::SplitChild(size_t pos)
{
    BTPage* pChild1 = nullptr;
    BTPage* pChild2 = nullptr;

    if (pos > 0 && m_SubPages[pos-1]->IsFull()) {
        pChild1 = m_SubPages[pos-1];
        pChild2 = m_SubPages[pos--];
    }
    if (pos < GetNumberOfKeys() && m_SubPages[pos+1]->IsFull()) {
        pChild1 = m_SubPages[pos];
        pChild2 = m_SubPages[pos+1];
    }

    vector<ObjectInfo> tmpKeys;
    vector<BTPage*>    tmpSubPages;

    MovePage(pChild1, tmpKeys, tmpSubPages);
    tmpKeys.push_back(m_Keys[pos]);
    MovePage(pChild2, tmpKeys, tmpSubPages);

    BTPage*    pChild3 = nullptr;
    ObjectInfo oi1, oi2;
    SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

    m_Keys[pos]    = oi1;
    m_SubPages[pos] = pChild1;

    ::insert_at(m_Keys,    oi2,    pos + 1);
    ::insert_at(m_SubPages, pChild2, pos + 1);
    NumberOfKeys()++;

    m_SubPages[pos + 2] = pChild3;
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::SplitPageInto3(
    vector<ObjectInfo>& tmpKeys,
    vector<BTPage*>&    tmpSubPages,
    BTPage*& pChild1, BTPage*& pChild2, BTPage*& pChild3,
    ObjectInfo& oi1,  ObjectInfo& oi2)
{
    assert(tmpKeys.size() >= 8);
    assert(tmpSubPages.size() >= 9);

    if (!pChild1) pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);
    pChild1->clear();

    size_t nKeys = (tmpKeys.size() - 2) / 3;
    size_t i = 0;
    for (; i < nKeys; i++) {
        pChild1->m_Keys    [i] = tmpKeys    [i];
        pChild1->m_SubPages[i] = tmpSubPages[i];
        pChild1->NumberOfKeys()++;
    }
    pChild1->m_SubPages[i] = tmpSubPages[i];
    oi1 = tmpKeys[i++];

    if (!pChild2) pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
    pChild2->clear();

    nKeys += (tmpKeys.size() - 2) / 3 + 1;
    size_t j = 0;
    for (; i < nKeys; i++, j++) {
        pChild2->m_Keys    [j] = tmpKeys    [i];
        pChild2->m_SubPages[j] = tmpSubPages[i];
        pChild2->NumberOfKeys()++;
    }
    pChild2->m_SubPages[j] = tmpSubPages[i];
    oi2 = tmpKeys[i++];

    if (!pChild3) pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
    pChild3->clear();

    nKeys = tmpKeys.size();
    for (j = 0; i < nKeys; i++, j++) {
        pChild3->m_Keys    [j] = tmpKeys    [i];
        pChild3->m_SubPages[j] = tmpSubPages[i];
        pChild3->NumberOfKeys()++;
    }
    pChild3->m_SubPages[j] = tmpSubPages[i];
}

template <typename keyType, typename ObjIDType>
bool CBTreePage<keyType, ObjIDType>::SplitRoot()
{
    BTPage*    pChild1 = nullptr, *pChild2 = nullptr, *pChild3 = nullptr;
    ObjectInfo oi1, oi2;
    SplitPageInto3(m_Keys, m_SubPages, pChild1, pChild2, pChild3, oi1, oi2);
    clear();

    m_Keys    [0] = oi1;
    m_SubPages[0] = pChild1;
    NumberOfKeys()++;

    m_Keys    [1] = oi2;
    m_SubPages[1] = pChild2;
    NumberOfKeys()++;

    m_SubPages[2] = pChild3;
    return true;
}

template <typename keyType, typename ObjIDType>
bool CBTreePage<keyType, ObjIDType>::Search(const keyType& key, ObjIDType& ObjID)
{
    size_t pos = binary_search(m_Keys, (size_t)0, m_KeyCount, key);
    if (pos >= m_KeyCount) {
        if (m_SubPages[pos]) return m_SubPages[pos]->Search(key, ObjID);
        return false;
    }
    if (key == m_Keys[pos].key) {
        ObjID = m_Keys[pos].ObjID;
        m_Keys[pos].UseCounter++;
        return true;
    }
    if (key < m_Keys[pos].key)
        if (m_SubPages[pos]) return m_SubPages[pos]->Search(key, ObjID);
    return false;
}

template <typename keyType, typename ObjIDType>
bt_ErrorCode CBTreePage<keyType, ObjIDType>::Remove(
    const keyType& key, const ObjIDType ObjID)
{
    bt_ErrorCode error = bt_ok;
    size_t pos = binary_search(m_Keys, (size_t)0, m_KeyCount, key);

    if (pos < NumberOfKeys() && key == m_Keys[pos].key) {
        if (!m_SubPages[pos + 1]) {
            ::remove_at(m_Keys, pos);
            NumberOfKeys()--;
            if (Underflow()) return bt_underflow;
            return bt_ok;
        }
        {
            ObjectInfo& rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
            swap(m_Keys[pos], rFirstFromRight);
            error = m_SubPages[++pos]->Remove(key, ObjID);
        }
    }
    else if (pos == NumberOfKeys()) {
        error = m_SubPages[pos]->Remove(key, ObjID);
    }
    else if (key <= m_Keys[pos].key) {
        if (m_SubPages[pos]) error = m_SubPages[pos]->Remove(key, ObjID);
        else return bt_nofound;
    }

    if (error == bt_underflow) {
        if (TreatUnderflow(pos)) return bt_ok;
        if (IsRoot() && NumberOfKeys() == 2) return MergeRoot();
        return Merge(pos);
    }
    if (error == bt_nofound) return bt_nofound;
    return bt_ok;
}

template <typename keyType, typename ObjIDType>
bt_ErrorCode CBTreePage<keyType, ObjIDType>::Merge(size_t pos)
{
    assert(m_SubPages[pos-1]->NumberOfKeys() +
           m_SubPages[pos  ]->NumberOfKeys() +
           m_SubPages[pos+1]->NumberOfKeys() ==
           3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

    vector<ObjectInfo> tmpKeys;
    vector<BTPage*>    tmpSubPages;

    BTPage* pChild1 = m_SubPages[pos-1];
    BTPage* pChild2 = m_SubPages[pos  ];
    BTPage* pChild3 = m_SubPages[pos+1];

    MovePage(pChild1, tmpKeys, tmpSubPages);
    tmpKeys.push_back(m_Keys[pos-1]);
    MovePage(pChild2, tmpKeys, tmpSubPages);
    tmpKeys.push_back(m_Keys[pos]);
    MovePage(pChild3, tmpKeys, tmpSubPages);
    pChild3->Destroy();

    size_t nKeys = pChild1->GetFreeCells();
    size_t i = 0;
    for (; i < nKeys; i++) {
        pChild1->m_Keys    [i] = tmpKeys    [i];
        pChild1->m_SubPages[i] = tmpSubPages[i];
        pChild1->NumberOfKeys()++;
    }
    pChild1->m_SubPages[i] = tmpSubPages[i];

    m_Keys    [pos-1] = tmpKeys[i];
    m_SubPages[pos-1] = pChild1;

    ::remove_at(m_Keys,    pos);
    ::remove_at(m_SubPages, pos);
    NumberOfKeys()--;

    nKeys = pChild2->GetFreeCells();
    size_t j = ++i;
    for (i = 0; i < nKeys; i++, j++) {
        pChild2->m_Keys    [i] = tmpKeys    [j];
        pChild2->m_SubPages[i] = tmpSubPages[j];
        pChild2->NumberOfKeys()++;
    }
    pChild2->m_SubPages[i] = tmpSubPages[j];
    m_SubPages[pos] = pChild2;

    if (Underflow()) return bt_underflow;
    return bt_ok;
}

template <typename keyType, typename ObjIDType>
bt_ErrorCode CBTreePage<keyType, ObjIDType>::MergeRoot()
{
    size_t pos = 1;
    assert(m_SubPages[pos-1]->NumberOfKeys() +
           m_SubPages[pos  ]->NumberOfKeys() +
           m_SubPages[pos+1]->NumberOfKeys() ==
           3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

    BTPage* pChild1 = m_SubPages[pos-1];
    BTPage* pChild2 = m_SubPages[pos  ];
    BTPage* pChild3 = m_SubPages[pos+1];
    size_t nKeys = pChild1->NumberOfKeys() +
                   pChild2->NumberOfKeys() +
                   pChild3->NumberOfKeys() + 2;

    vector<ObjectInfo> tmpKeys;
    vector<BTPage*>    tmpSubPages;

    MovePage(pChild1, tmpKeys, tmpSubPages);
    tmpKeys.push_back(m_Keys[pos-1]);
    MovePage(pChild2, tmpKeys, tmpSubPages);
    tmpKeys.push_back(m_Keys[pos]);
    MovePage(pChild3, tmpKeys, tmpSubPages);

    clear();
    size_t i = 0;
    for (; i < nKeys; i++) {
        m_Keys    [i] = tmpKeys    [i];
        m_SubPages[i] = tmpSubPages[i];
        NumberOfKeys()++;
    }
    m_SubPages[i] = tmpSubPages[i];

    pChild1->Destroy();
    pChild2->Destroy();
    pChild3->Destroy();

    return bt_rootmerged;
}

template <typename keyType, typename ObjIDType>
typename CBTreePage<keyType, ObjIDType>::ObjectInfo&
CBTreePage<keyType, ObjIDType>::GetFirstObjectInfo()
{
    if (m_SubPages[0]) return m_SubPages[0]->GetFirstObjectInfo();
    return m_Keys[0];
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::Print(ostream& os)
{
    // Reutiliza ForEach variadic con una lambda
    // (ya no existe la función global Print estilo C)
    ForEach([](tagObjectInfo<keyType, ObjIDType>& info,
               size_t level, ostream& out)
    {
        for (size_t i = 0; i < level; i++) out << "\t";
        out << info.key << "->" << info.ObjID << "\n";
    }, (size_t)0, os);
}

template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::MovePage(
    BTPage* pChildPage,
    vector<ObjectInfo>& tmpKeys,
    vector<BTPage*>&    tmpSubPages)
{
    size_t nKeys = pChildPage->GetNumberOfKeys();
    size_t i = 0;
    for (; i < nKeys; i++) {
        tmpKeys    .push_back(pChildPage->m_Keys    [i]);
        tmpSubPages.push_back(pChildPage->m_SubPages[i]);
    }
    tmpSubPages.push_back(pChildPage->m_SubPages[i]);
    pChildPage->clear();
}

template <typename keyType, typename ObjIDType>
size_t CBTreePage<keyType, ObjIDType>::GetFreeCellsOnLeft(size_t pos)
{
    if (pos > 0) return m_SubPages[pos-1]->GetFreeCells();
    return 0;
}

template <typename keyType, typename ObjIDType>
size_t CBTreePage<keyType, ObjIDType>::GetFreeCellsOnRight(size_t pos)
{
    if (pos < GetNumberOfKeys()) return m_SubPages[pos+1]->GetFreeCells();
    return 0;
}

#endif // CBTreePage_H