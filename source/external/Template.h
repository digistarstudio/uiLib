//---------------------------------------------------------------------------------------------------------------------
//
// File name   : Template.h
// Author      : Created by Pointer.
// State       : 
// Update date : 06/03/06
// Description : 
// Caution     : TSimpleArray can't be used with the class that has vtable.
//
//---------------------------------------------------------------------------------------------------------------------


#pragma once


#include "UTX.h"


template<class T> class TSimpleArray
{
public:

	INLINE TSimpleArray(UINT defaultsize, UINT incslot = 10)
	{
		MaxSlot = defaultsize;
		IncSlot = incslot;

		if(MaxSlot)
			pData = Allocate();
		else
			pData = NULL;
	}
	INLINE ~TSimpleArray()
	{
		Free();
	}

	INLINE T& operator()(UINT index)
	{
		// Make sure index is valid.
		ASSERT(index < MaxSlot);
		return Data[index];
	}

	INLINE void SetIncSize(UINT size)
	{
		IncSlot = size;
	}

	INLINE void SetMaxSize(UINT newsize)
	{
		MaxSlot = newsize;
		Reallocate();
	}

	INLINE void ResizeBuffer(BOOL inc = TRUE)
	{
		if(inc)
			MaxSlot += IncSlot;
		Reallocate();
	}

	INLINE void ReleaseBuffer()
	{
		Free();
		MaxSlot = 0;
	}

	TSimpleArray<T>& operator=(TSimpleArray<T> &obj)
	{
		if(MaxSlot != obj.MaxSlot)
		{
			Free();
			MaxSlot = obj.MaxSlot;
			pData = Allocate();
		}
		IncSlot = obj.IncSlot;

		UINT i;
		for(i = 0; i < MaxSlot; i++)
			pData[i] = obj.pData[i];

		return *this;
	}

	UINT GetMaxSlot() const
	{
		return MaxSlot;
	}


protected:

	UINT MaxSlot;
	UINT IncSlot;
	T    *pData;


private:

	INLINE T* Allocate()
	{
		return (T*)malloc(MaxSlot * sizeof(T));
	}

	INLINE void Reallocate()
	{
		pData = (T*)realloc(pData, MaxSlot * sizeof(T));
		ASSERT(pData);
	}

	INLINE void Free()
	{
		if(pData)
		{
			free(pData);
			pData = 0;
		}
	}


};


typedef void* ListIndex;


template<class T> class TList
{
public:

	TList(UINT MaxCacheCounts = 0)
	{
		INIT_LIST_HEAD(&m_ListHead);
		m_Counts = 0;
		m_MaxCacheCounts = MaxCacheCounts;
		m_CCacheCounts = 0;
		m_pCacheHead = nullptr;
	}
	TList(TList<T> &in)
	{
	}
	~TList()
	{
		list_entry *temp, *next = LIST_GET_HEAD(m_ListHead);
		while (IS_VALID_ENTRY(next, m_ListHead))
		{
			temp = next->next;
			delete (sNode*)next;
			next = temp;
			m_Counts--;
		}
		ASSERT(!m_Counts);

		sNode *stemp = m_pCacheHead;
		while (stemp != nullptr)
		{
			m_pCacheHead = (sNode*)stemp->m_ListEntry.next;
			delete stemp;
			stemp = m_pCacheHead;
			m_CCacheCounts--;
		}
		ASSERT(!m_CCacheCounts);
	}

	ListIndex AddData(T &data) // Add data to tail of list.
	{
		// Add data to the tail of list.
		sNode *newnode = NewNode();
		newnode->Data = data;
		list_add_tail((list_entry*)newnode, &m_ListHead);
		m_Counts++;
		return (void*)newnode;
	}

	ListIndex AddDataHead(T &data) // Add data to head of list.
	{
		// Add data to the tail of list.
		sNode *newnode = NewNode();
		newnode->Data = data;
		list_add((list_entry*)newnode, &m_ListHead);
		m_Counts++;
		return (void*)newnode;
	}

	T& GetDataR(ListIndex index)
	{
		return ((sNode*)index)->Data;
	}

	T& GetDataRN(ListIndex &index)
	{
		T &ReturnValue = ((sNode*)index)->Data;
		index = ((sNode*)index)->pNext;
		return ReturnValue;
	}

	T* GetDataP(ListIndex index)
	{
		return &(((sNode*)index)->Data);
	}

	T* GetDataPN(ListIndex &index)
	{
		T* ReturnValue = &(((sNode*)index)->Data);
		index = ((list_entry*)index)->next;
		return ReturnValue;
	}

	//void IndexNext(ListIndex *index)
	//{
	//	*index = ((sNode*)(*index))->pNext;
	//}

	//void IndexPrev(ListIndex *index)
	//{
	//	*index = ((sNode*)(*index))->pPrev;
	//}

	INLINE ListIndex GetHeadIndex()
	{
		return IS_VALID_ENTRY(LIST_GET_HEAD(m_ListHead), m_ListHead) ? LIST_GET_HEAD(m_ListHead) : nullptr;
	}

	INLINE ListIndex GetTailIndex()
	{
		return IS_VALID_ENTRY(LIST_GET_TAIL(m_ListHead), m_ListHead) ? LIST_GET_TAIL(m_ListHead) : nullptr;
	}

	// The data you get will be removed from the list and index will be next one.
	void GetAndRemove(ListIndex &index, T &out)
	{
		sNode* cnode = (sNode*)index;
		out = cnode->Data;
		index = (ListIndex)cnode->m_ListEntry.next;
		list_remove((list_entry*)cnode);
		DelNode(cnode);
		m_Counts--;
	}

	void RemoveNode(ListIndex index)
	{
		ASSERT(((sNode*)index)->pOwnerList == this);
		list_remove((list_entry*)index);
		m_Counts--;
		DelNode((sNode*)index);
	}

	void Release()
	{
		if(m_Counts)
		{
			DelNode(m_pHead, m_pTail, m_Counts);
			INIT_LIST_HEAD(&m_ListHead);
			m_Counts = 0;
		}
	}

	void SmartRemoveNode(ListIndex index)
	{
		((sNode*)index)->pOwnerList->RemoveNode(index);
	}

	INLINE UINT GetCounts()
	{
		return m_Counts;
	}

	void ZeroInit()
	{
		m_Counts = 0;
		INIT_LIST_HEAD(&m_ListHead);
	}

	INLINE void SetMaxCacheCounts(UINT counts)
	{
		m_MaxCacheCounts = counts;
	}

	void TakeOver(TList<T> &source)
	{
		if(!source.m_Counts)
			return;

		// Reset list node's owner.
		sNode* temp = source.m_pHead;
		while(temp)
		{
			temp->pOwnerList = this;
			temp = temp->pNext;
		}

		if(m_Counts)
		{
			source.m_pHead->pPrev = m_pTail;
			m_pTail->pNext = source.m_pHead;
			m_pTail = source.m_pTail;
			m_Counts += source.m_Counts;
		}
		else
		{
			m_pHead = source.m_pHead;
			m_pTail = source.m_pTail;
			m_Counts = source.m_Counts;
		}
		source.ZeroInit();
	}


private:

	const TList<T>& operator=(TList<T> &source)
	{
		Release();

		// Take over source's data.
		m_Counts = source.m_Counts;
		m_pHead = source.m_pHead;
		m_pTail = source.m_pTail;

		// Reset list node's owner.
		sNode* temp = m_pHead;
		while(temp)
		{
			temp->pOwnerList = this;
			temp = temp->pNext;
		}

		source.ZeroInit();
		return *this;
	}


protected:

	struct sNode
	{
		sNode()
		:pOwnerList(nullptr)
		{
			INIT_LIST_ENTRY(&m_ListEntry);
		}

		// Data member.
		list_entry m_ListEntry;
		TList<T> *pOwnerList;
		T Data;         // This must be last. Don't change layout.

	};

	INLINE sNode* NewNode()
	{
		sNode* temp;
		if (m_pCacheHead == nullptr)
		{
			ASSERT(!m_CCacheCounts);
			temp = new sNode;
			temp->pOwnerList = this;
		}
		else
		{
			temp = m_pCacheHead;
			m_pCacheHead = (sNode*)temp->m_ListEntry.next;
			INIT_LIST_ENTRY((list_entry*)temp);
			m_CCacheCounts--;
		}

		return temp;
	}

	INLINE void DelNode(sNode *pNode)
	{
		if(m_CCacheCounts >= m_MaxCacheCounts)
			delete pNode;
		else
		{
			pNode->m_ListEntry.next = (list_entry*)m_pCacheHead;
			m_pCacheHead = pNode;
			m_CCacheCounts++;
		}
	}

	INLINE void DelNode(sNode *pHeadNode, sNode *pTailNode, INT Counts)
	{
#ifdef DEBUG
		INT Counter = Counts;
		sNode *pNode = pHeadNode;
		while(--Counter)
			pNode = pNode->pNext;
		ASSERT(pNode == pTailNode && !pNode->pNext);
#endif

		sNode *pTemp = pTailNode;
		if (Counts + m_CCacheCounts > m_MaxCacheCounts)
		{
			INT remove = Counts + m_CCacheCounts - m_MaxCacheCounts;
			Counts -= remove;

			while (remove--)
			{
				pTemp = pTailNode->m_ListEntry.prev;
				delete pTailNode;
				pTailNode = pTemp;
			}
		}

		if (pTailNode != nullptr) // Chech if all nodes are removed.
		{
			pTailNode->m_ListEntry.next = m_pCacheHead;
			m_pCacheHead = pHeadNode;
			m_CCacheCounts += Counts;
		}
	}


protected:

	list_head m_ListHead;

	UINT   m_Counts;
	UINT   m_MaxCacheCounts, m_CCacheCounts;
	sNode  *m_pCacheHead;

	friend void ListIndexNext(ListIndex&);
	friend void ListIndexPrev(ListIndex&);


};


INLINE void ListIndexNext(ListIndex &index)
{
	ASSERT(index);
	ASSERT(IS_VALID_ENTRY(index, ((TList<INT>::sNode*)index)->pOwnerList->m_ListHead));
	index = ((TList<INT>::sNode*)index)->m_ListEntry.next;
}

INLINE void ListIndexPrev(ListIndex &index)
{
	ASSERT(index);
	index = ((TList<INT>::sNode*)index)->m_ListEntry.next;
}


