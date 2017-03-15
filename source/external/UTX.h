//---------------------------------------------------------------------------------------------------------------------
//
// File name   : UTX.h
// Author      : Created by Pointer.
// State       : 
// Update date : 06/01/16
// Description : 
//
//---------------------------------------------------------------------------------------------------------------------


#ifndef __HEADER_UTILITIES__H__
#define __HEADER_UTILITIES__H__


#include <type_traits>
#include <assert.h>
//#include "RMath.h"


#ifndef ASSERT
#define ASSERT assert
#endif

#ifdef _DEBUG
	#define VERIFY(statement) ASSERT(statement)
#else
	#define VERIFY(statement) ((void)(statement))
#endif

#define INLINE inline

#define IMPLEMENT_ENUM_FLAG(ENUM_NAME) \
INLINE ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<INT>(a) | static_cast<INT>(b)); \
} \
INLINE ENUM_NAME& operator|=(ENUM_NAME& a, ENUM_NAME b) \
{ \
	return a = static_cast<ENUM_NAME>(static_cast<INT>(a) | static_cast<INT>(b)); \
}

#define TYPE_MAX_VALUE(variable) (std::numeric_limits<typeid(variable)>::max())
#define TYPE_MIN_VALUE(variable) (std::numeric_limits<typeid(variable)>::lowest())


typedef signed char      INT8, *PINT8;
typedef signed short     INT16, *PINT16;
typedef signed int       INT32, *PINT32;
typedef signed __int64   INT64, *PINT64;
typedef unsigned char    UINT8, *PUINT8;
typedef unsigned short   UINT16, *PUINT16;
typedef unsigned int     UINT32, *PUINT32;
typedef unsigned __int64 UINT64, *PUINT64;
typedef double           DOUBLE;

#define MAX_UINT64_VALUE 18446744073709551615
#define MAX_UINT_VALUE   4294967295
#define MAX_INT_VALUE    2147483647
#define MIN_INT_VALUE    (-2147483647 - 1)
#define MAX_BYTE_VALUE   255

#define PI 3.14159265358979323846
#define ZeroBias 0.01f


#define MEMBER_SIZE(type, field) (sizeof( ((type*)0)->field ))

#define SAFE_DELETE_ARRAY(ptr) { if ((ptr) != nullptr) { delete[] (ptr); (ptr) = nullptr; } }
#define SAFE_DELETE(ptr) { if ((ptr) != nullptr) { delete (ptr); (ptr) = nullptr; } }
#define SAFE_RELEASE(ptr) { if ((ptr) != nullptr) { (ptr)->Release(); (ptr) = nullptr; } }

#define UTXMAX(a, b) ((a) > (b)? (a) : (b))
#define UTXMIN(a, b) ((a) < (b)? (a) : (b))

#define BEGIN_NAMESPACE(name) namespace name {
#define END_NAMESPACE }

#define IS_ALIGNED(addr, byte_count) IsAligned((void*)(addr), (byte_count))
#define POINTER_IS_ALIGNED(addr)     IsAligned((void*)(addr), sizeof(void*))

inline BOOL IsAligned(const void *__restrict address, size_t byte_count)
{
	return ((uintptr_t)address % byte_count) == 0;
}


#if defined(DEBUG_ATOMIC_FUNCTION) || defined(_DEBUG)

template<class T>
__forceinline T InterlockedIncrementT(T volatile* addr)
{
	ASSERT(sizeof(T) == 4 && IS_ALIGNED(addr, 4)); // 32-bit only.
	return (T)InterlockedIncrement((LONG volatile*)addr);
}

template<class T>
__forceinline T InterlockedDecrementT(T volatile* addr)
{
	ASSERT(sizeof(T) == 4 && IS_ALIGNED(addr, 4)); // 32-bit only.
	return (T)InterlockedDecrement((LONG volatile*)addr);
}

template<class T>
__forceinline T* InterlockedExchangePointerT(T *volatile* PtrAddr, void *Value)
{
	ASSERT(POINTER_IS_ALIGNED(PtrAddr));
	return (T*)InterlockedExchangePointer((void*volatile*)PtrAddr, Value);
}

template<class T>
__forceinline T* InterlockedCompareExchangePointerT(T *volatile* PtrAddr, void *value, void *comparand)
{
	ASSERT(POINTER_IS_ALIGNED(PtrAddr));
	return (T*)InterlockedCompareExchangePointer((void *volatile*)PtrAddr, value, comparand);
}

template<class T>
__forceinline T InterlockedExchangeT(T volatile* addr, T value)
{
	ASSERT(sizeof(T) == 4);
	return (T)InterlockedExchange((LONG volatile*)addr, value);
}

#else

template<class T>
__forceinline T InterlockedIncrementT(T volatile* addr)
{
	return (T)InterlockedIncrement((LONG volatile*)addr);
}

template<class T>
__forceinline T InterlockedDecrementT(T volatile* addr)
{
	return (T)InterlockedDecrement((LONG volatile*)addr);
}

template<class T>
__forceinline T* InterlockedExchangePointerT(T *volatile* PtrAddr, void *Value)
{
	return (T*)InterlockedExchangePointer((void*volatile*)PtrAddr, Value);
}

template<class T>
__forceinline T* InterlockedCompareExchangePointerT(T *volatile* PtrAddr, void *value, void *comparand)
{
	return (T*)InterlockedCompareExchangePointer((void *volatile*)PtrAddr, value, comparand);
}

template<class T>
__forceinline T InterlockedExchangeT(T volatile* addr, T value)
{
	return (T)InterlockedExchangeT(addr, value);
}

#endif // Debug


BOOL UTXLibraryInit(); // Don't call it in constructor of a global object.
void UTXLibraryEnd();
int printx(const char * format, ...);
int printx(const wchar_t * format, ...);


// String function.
void UTXStringLowercase(TCHAR *in);
void UTXStringUppercase(TCHAR *in);
BOOL UTXIsStringLowercase(TCHAR const *in);
BOOL UTXIsStringUppercase(TCHAR const *in);



INLINE FLOAT Abs(FLOAT in)
{
	if(in < 0)
		return in * -1;
	return in;
}

INLINE DOUBLE Abs(DOUBLE in)
{
	if(in < 0)
		return in * -1;
	return in;
}

INLINE INT Abs(INT in)
{
	if(in < 0)
		return in * -1;
	return in;
}
/*
INLINE sAABB AABBAnd(sAABB &aabb1, sAABB &aabb2)
{
	sAABB temp;

	temp.MaxX = (aabb1.MaxX > aabb2.MaxX) ? aabb1.MaxX : aabb2.MaxX;
	temp.MaxY = (aabb1.MaxY > aabb2.MaxY) ? aabb1.MaxY : aabb2.MaxY;
	temp.MaxZ = (aabb1.MaxZ > aabb2.MaxZ) ? aabb1.MaxZ : aabb2.MaxZ;

	temp.MinX = (aabb1.MinX < aabb2.MinX) ? aabb1.MinX : aabb2.MinX;
	temp.MinY = (aabb1.MinY < aabb2.MinY) ? aabb1.MinY : aabb2.MinY;
	temp.MinZ = (aabb1.MinZ < aabb2.MinZ) ? aabb1.MinZ : aabb2.MinZ;

	return temp;
}

INLINE BOOL IsVector3Equal(XMFLOAT3 &vect1, XMFLOAT3 &vect2, const FLOAT epsilon = 0.075f)
{
	XMFLOAT3 dist = vect1 - vect2;

	if(fabs(dist.x) < epsilon && fabs(dist.y) < epsilon && fabs(dist.z) < epsilon)
		return TRUE;

	return FALSE;
}
*/
INLINE FLOAT SetRange(FLOAT min, FLOAT max, FLOAT in)
{
	if(in < min)
		return min;
	else if(in > max)
		return max;
	else
		return in;
}

INLINE void UTXGetWorkingPath(TCHAR *buffer, DWORD size)
{
	GetModuleFileName(0, buffer, size);
}


INLINE void UTXGetFileExtName(TCHAR const *src, TCHAR *buffer, size_t &bsize)
{
	size_t osize = _tcslen(src);
	size_t temp = osize;

	while(temp--)
		if(src[temp] == '.')
			break;

	if(!temp) // File doesn't exists extended name.
	{
		bsize = 0;
		return;
	}

	if(osize - temp > bsize)
	{
		bsize = osize - temp;
		return;
	}

	bsize = osize - temp;
	memcpy(buffer, &src[temp + 1], sizeof(TCHAR) * bsize);
}


INLINE size_t UTXGetFileNamePos(TCHAR const *src)
{
	size_t osize = _tcslen(src);
	size_t temp = osize;

	while(temp--)
		if(src[temp] == '\\')
			break;

	return temp + 1;
}


//-------------------------------------------------------------------------------------------------
// Unicode & Ansi covert functions.
// If bsize is zero, the return value is the required buffer size, in
// bytes, for a buffer that can receive the translated string.
//-------------------------------------------------------------------------------------------------

INLINE BOOL IsUnicode()
{
#if defined(UNICODE) || defined(_UNICODE)
	return TRUE;
#else
	return FALSE;
#endif
}

INLINE INT UTXUnicodeToAnsi(WCHAR *string, CHAR *buffer, INT bsize)
{
	ZeroMemory(buffer, bsize);
	return WideCharToMultiByte(CP_ACP, 0, string, -1, buffer, bsize, nullptr, nullptr);
}

INLINE INT UTXAnsiToUnicode(CHAR *string, WCHAR *buffer, INT bsize)
{
	return MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, bsize);
}


struct sRect
{
	sRect()
	{
	}

	INT Width()
	{
		return right - x;
	}

	INT Height()
	{
		return bottom - y;
	}

	INT x, y, right, bottom;


};


class CBitSet
{
public:

	CBitSet()
	:m_bUseDynamicBuffer(FALSE), m_ArraySize(0), m_Data(0), m_pData(nullptr)
	{
	}
	~CBitSet()
	{
		if(m_pData != nullptr)
			free(m_pData);
	}

	enum{ UINT_BIT_COUNT = sizeof(UINT64) * 8 };

	void Allocate(UINT bitcounts)
	{
		ASSERT(bitcounts > 0);

		if(bitcounts > UINT_BIT_COUNT)
		{
			m_bUseDynamicBuffer = TRUE;
			UINT ArraySize = bitcounts / UINT_BIT_COUNT + 1;
			if(m_ArraySize)
			{
				if(m_ArraySize < ArraySize)
				{
					m_pData = (UINT*)realloc(m_pData, sizeof(UINT) * ArraySize);
					m_ArraySize = ArraySize;
				}
			}
			else
			{
				m_pData = (UINT*)malloc(sizeof(UINT) * ArraySize);
				m_ArraySize = ArraySize;
			}
		}
		else
			m_bUseDynamicBuffer = FALSE;

		// Init all bit with zero.
		CleanAllBit();
	}

	void SetTrueBit(UINT BitIndex)
	{
		DWORD mask = 1 << (BitIndex % UINT_BIT_COUNT);

		if(m_bUseDynamicBuffer)
			m_pData[BitIndex / UINT_BIT_COUNT] |= mask;
		else
			m_Data |= mask;
	}

	void SetFalseBit(UINT BitIndex)
	{
		DWORD mask = ~(1 << (BitIndex % UINT_BIT_COUNT));

		if(m_bUseDynamicBuffer)
			m_pData[BitIndex / UINT_BIT_COUNT] &= mask;
		else
			m_Data &= mask;
	}

	BOOL GetBit(UINT BitIndex)
	{
		DWORD mask = 1 << (BitIndex % UINT_BIT_COUNT);

		if(m_bUseDynamicBuffer)
			return m_pData[BitIndex / UINT_BIT_COUNT] & mask;

		return m_Data & mask;
	}

	void CleanAllBit()
	{
		if(m_bUseDynamicBuffer)
			ZeroMemory(m_pData, sizeof(UINT) * m_ArraySize);
		else
			m_Data = 0;
	}


protected:

	BOOL m_bUseDynamicBuffer;
	UINT m_ArraySize;
	UINT64 m_Data;   // Static for small set.
	UINT   *m_pData; // Dynamic memory space.


};


class CPerformanceCounter
{
public:

	__forceinline void Start()
	{
		QueryPerformanceCounter(&t);
	}
	__forceinline void End()
	{
		LARGE_INTEGER et;
		QueryPerformanceCounter(&et);
		t.QuadPart = et.QuadPart - t.QuadPart;
	}

	__forceinline DOUBLE Get(LARGE_INTEGER f)
	{
		return (DOUBLE)t.QuadPart / f.QuadPart * 1000;
	}
	__forceinline DOUBLE Get()
	{
		return (DOUBLE)t.QuadPart * factor; // This won't lose precision. (format token: %f)
	}

	__forceinline static LARGE_INTEGER GetFreq() { return freq; }


protected:

	friend BOOL UTXLibraryInit();

	static LARGE_INTEGER freq;
	static DOUBLE factor;

	LARGE_INTEGER t;


};


class CCriticalSectionUTX
{
public:

	CCriticalSectionUTX(DWORD dwSpinCount = 0, DWORD Flags = 0) { ::InitializeCriticalSectionAndSpinCount(&m_cs, dwSpinCount); }
	~CCriticalSectionUTX() { ::DeleteCriticalSection(&m_cs); }

	DWORD SetCriticalSectionSpinCount(DWORD dwSpinCount) { return ::SetCriticalSectionSpinCount(&m_cs, dwSpinCount); }
	BOOL  TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) { return ::TryEnterCriticalSection(&m_cs); }

	void EnterCriticalSection() { ::EnterCriticalSection(&m_cs); }
	void LeaveCriticalSection() { ::LeaveCriticalSection(&m_cs); }


protected:

	CRITICAL_SECTION m_cs;


};


typedef struct _list_entry
{
	_list_entry *prev, *next;
} list_entry, list_head;


#define INIT_LIST_HEAD(p) {(p)->prev = (p)->next = p;}
#define INIT_LIST_ENTRY(p) {(p)->prev = (p)->next = nullptr;}
#define IS_VALID_ENTRY(p, head) ((list_entry*)(p) != &(head))
#define LIST_IS_EMPTY(head) ((head).next == &(head)) // Must test next pointer to be compatible with list_add_single.
#define IS_NULL_ENTRY(entry) ((entry).next == nullptr)
#define LIST_GET_HEAD(head) ((head).next)
#define LIST_GET_TAIL(head) ((head).prev)

// Insert a new entry between two known consecutive entries.
// This is only for internal list manipulation where we know the prev/next entries already!
static inline void __list_add(list_entry *newnode, list_entry *prev, list_entry *next)
{
	next->prev = newnode;
	newnode->next = next;
	newnode->prev = prev;
	prev->next = newnode;
}

static inline void __list_del(list_entry *prev, list_entry *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline INT list_count(list_head *phead)
{
	INT i = 0;
	for (list_entry *entry = phead->next; entry != phead; entry = entry->next, ++i);
	return i;
}

static inline void list_add(list_entry *newnode, list_head *head)
{
	__list_add(newnode, head, head->next);
}
static inline void list_add_tail(list_entry *newnode, list_head *head)
{
	__list_add(newnode, head->prev, head);
}
static inline void list_insert_after(list_entry *newnode, list_entry *pos)
{
	__list_add(newnode, pos, pos->next);
}
static inline void list_insert_before(list_entry *newnode, list_entry *pos)
{
	__list_add(newnode, pos->prev, pos);
}
static inline void list_remove(list_entry *entry)
{
	__list_del(entry->prev, entry->next);
}
static inline void list_add_single(list_entry *newnode, list_head *head) // Single direction link. (FILO)
{
	newnode->next = head->next;
	head->next = newnode;
}


class XStack
{
public:

	enum
	{
		OPERATION_LOCK_VALUE = 1, // Must sure no object is in this address.
	};

	XStack()
	{
		m_pStackHead = nullptr;
		ASSERT(IsAligned((void*)&m_pStackHead, sizeof(void*)));
	}

	struct stRefObj
	{
		void *pNext;
	};

	void* PopObject()
	{
		stRefObj *pRefObj;

		for (; ;)
		{
			pRefObj = (stRefObj*)InterlockedExchangePointerT(&m_pStackHead, (void*)OPERATION_LOCK_VALUE);
			if (!pRefObj)
			{
				m_pStackHead = nullptr;
				break;
			}
			if (pRefObj == (void*)OPERATION_LOCK_VALUE)
				continue;

			m_pStackHead = pRefObj->pNext;
			return pRefObj;
		}

		return nullptr;
	}

	void PushObj(void *pObjEntry)
	{
		stRefObj *pRefHead;
		//	ASSERT(IsAddressValid(pObjEntry));

		for (; ;)
		{
			pRefHead = (stRefObj*)m_pStackHead;
			if (pRefHead == (stRefObj*)OPERATION_LOCK_VALUE) // One thead calls GetFreeObj and runs into protected code.
				continue;

			((stRefObj*)pObjEntry)->pNext = pRefHead;
			if (InterlockedCompareExchangePointerT(&m_pStackHead, pObjEntry, pRefHead) == pRefHead)
				break;
		}
	}


protected:

	 void *volatile m_pStackHead;


};


/*
// This queue is thread-safe with multiple producer, but works only with one consumer.
// It's user's responsibility to keep enough pointer buffer, or cause bug when two new data wait for the same pointer.
*/

class XQueue
{
public:

	enum { QUEUE_SIZE = 64 };

	XQueue()
	{
		UnusedPos = ReadPos = 0;
		ZeroMemory(data, sizeof(data));

		ASSERT(IsAligned((void*)&UnusedPos, sizeof(UnusedPos)));
		ASSERT(IsAligned(data, sizeof(void*)));
	}

	void Add(void *dataIn)
	{
		UINT pos, nextpos;
		for (;;)
		{
			pos = UnusedPos;
			nextpos = (pos == QUEUE_SIZE - 1) ? 0 : pos + 1;
			if (InterlockedCompareExchange((volatile LONG*)&UnusedPos, nextpos, pos) != pos)
				continue;
			while (data[pos] != nullptr);
			data[pos] = dataIn;
			break;
		}
	}
	void* Get() // One consumer only.
	{
		void *pOut = (void*)data[ReadPos];
		if (pOut == nullptr)
			return nullptr;
		data[ReadPos] = nullptr;
		if (++ReadPos == QUEUE_SIZE)
			ReadPos = 0;
		return pOut;
	}


protected:

	volatile UINT UnusedPos, ReadPos;
	volatile void *data[QUEUE_SIZE];


};


template <class T>
class TObjectPool
{
public:

	TObjectPool()
	:m_nPool(0), m_nObjCountPerPool(0), m_pPoolArray(nullptr)
	{
	}
	~TObjectPool()
	{
		ASSERT(m_pPoolArray == nullptr);
	}

	enum
	{
		OPERATION_LOCK_VALUE = 1, // Must sure no object is in this address.
		DEFAULT_POOL_COUNT = 20,
	};


	struct stObjPool;

	struct stObjEntry
	{
		stObjPool *pPool;
		union
		{
			stObjEntry *pNext;
			T TData;
		};
	};

	struct stObjPool
	{
		//	stObjPool() {}   // This obj is allocated by malloc so constructor will never be called.
		//	~stObjPool() {}

		void Init(UINT nArraySize, UINT nTotalSize)
		{
			m_nPoolSize = nTotalSize;
			m_nArraySize = nArraySize--;

			stObjEntry *pBaseEntry = GetBaseEntryAddress(), *pEntry = pBaseEntry;
			for (; nArraySize--; ++pEntry)
			{
				ASSERT(pEntry != (stObjEntry*)OPERATION_LOCK_VALUE);
				pEntry->pNext = pEntry + 1;
				pEntry->pPool = this;
			}
			pEntry->pNext = nullptr;
			pEntry->pPool = this;
			m_pStackHead = pBaseEntry;
		}

		T* GetFreeObj()
		{
			for (stObjEntry *pRefObj; ;)
			{
				pRefObj = InterlockedExchangePointerT(&m_pStackHead, (void*)OPERATION_LOCK_VALUE);
				if (pRefObj == nullptr)
				{
					m_pStackHead = nullptr;
					break;
				}
				if (pRefObj == (stObjEntry*)OPERATION_LOCK_VALUE)
					continue;

				m_pStackHead = pRefObj->pNext;
				return (T*)&pRefObj->pNext;
			}

			return nullptr;
		}

		void RecycleObj(stObjEntry *pObjEntry)
		{
			stObjEntry *pRefHead;
			ASSERT(IsAddressValid(pObjEntry));

			for (; ;)
			{
				pRefHead = (stObjEntry*)m_pStackHead;
				if (pRefHead == (stObjEntry*)OPERATION_LOCK_VALUE) // One thead calls GetFreeObj and runs into protected code.
					continue;

				pObjEntry->pNext = pRefHead;
				if (InterlockedCompareExchangePointerT(&m_pStackHead, pObjEntry, pRefHead) == pRefHead)
					break;
			}
		}

		UINT CountFreeObj()
		{
			UINT nCount = 0;
			stObjEntry *pRefHead, *pObj;

			for (; ;)
			{
				pRefHead = InterlockedExchangePointerT(&m_pStackHead, (void*)OPERATION_LOCK_VALUE);
				if (pRefHead == nullptr)
				{
					m_pStackHead = nullptr;
					break;
				}
				if (pRefHead == (stObjEntry*)OPERATION_LOCK_VALUE)
					continue;

				for (pObj = pRefHead; pObj; ++nCount)
					pObj = pObj->pNext;

				m_pStackHead = pRefHead;
				break;
			}

			return nCount;
		}

		INLINE UINT GetIndex(stObjEntry *pObjEntry)
		{
			ASSERT(IsAddressValid(pObjEntry));
			return ((DWORD_PTR)pObjEntry - (DWORD_PTR)GetBaseEntryAddress()) / sizeof(stObjEntry);
		}
		BOOL IsAddressValid(stObjEntry *pEntry)
		{
			stObjEntry *pBase = GetBaseEntryAddress(), *pLastObj = pBase + (m_nArraySize - 1);
			if (pBase <= pEntry && pEntry <= pLastObj && (((DWORD_PTR)pEntry - (DWORD_PTR)pBase) % sizeof(stObjEntry)) == 0)
				return TRUE;
			return FALSE;
		}
		INLINE stObjEntry* GetBaseEntryAddress() { return pBase; }

		UINT m_nArraySize, m_nPoolSize;
		stObjEntry *volatile m_pStackHead;
		stObjEntry pBase[1]; // This must be the last data.

	};


	T* AllocObj()
	{
		T *pOut;
		for (UINT i = 0; i < m_nPool; ++i)
		{
			if (m_pPoolArray[i] == nullptr && !AddPool(i))
				break;
			if ((pOut = m_pPoolArray[i]->GetFreeObj()) != nullptr)
				return pOut;
		}
		return nullptr;
	}
	void FreeObj(T* pObj)
	{
		stObjEntry *pObjEntry = CONTAINING_RECORD(pObj, stObjEntry, pNext);
		pObjEntry->pPool->RecycleObj(pObjEntry);
	}
	T* NewObj()
	{
		T *pOut;
		for (UINT i = 0; i < m_nPool; ++i)
		{
			if (m_pPoolArray[i] == nullptr && !AddPool(i))
				break;
			if (pOut = m_pPoolArray[i]->GetFreeObj())
			{
				new(pOut) T();
				return pOut;
			}
		}
		return nullptr;
	}
	void DeleteObj(T* pObj)
	{
		stObjEntry *pObjEntry = CONTAINING_RECORD(pObj, stObjEntry, pNext);
		pObj->~T();
		pObjEntry->pPool->RecycleObj(pObjEntry);
	}


	BOOL CreatePool(UINT nObjCountPerPool, UINT nMaxPoolCount = DEFAULT_POOL_COUNT)
	{
		ASSERT(m_pPoolArray == nullptr);
		m_nObjCountPerPool = nObjCountPerPool;
		m_nPool = nMaxPoolCount;

		UINT nSize = sizeof(stObjPool*) * m_nPool;
		m_pPoolArray = (stObjPool**)malloc(nSize);
		if (m_pPoolArray == nullptr)
			return FALSE;
		ZeroMemory(m_pPoolArray, nSize);

		return TRUE;
	}
	void ReleasePool()
	{
		if (m_pPoolArray == nullptr)
			return;
		for (UINT i = 0; i < m_nPool; i++)
			if (m_pPoolArray[i] != nullptr)
				_aligned_free(m_pPoolArray[i]);
		free(m_pPoolArray);
		m_pPoolArray = nullptr;
		m_nPool = 0;
	}

	stObjPool* AddPool(UINT nIndex)
	{
		stObjPool *pPool = nullptr;
		m_cs.EnterCriticalSection();
		if (m_pPoolArray[nIndex] == nullptr)
			if ((pPool = AllocPool(m_nObjCountPerPool)) != nullptr)
				m_pPoolArray[nIndex] = pPool;
		m_cs.LeaveCriticalSection();
		return pPool;
	}

	stObjPool* AllocPool(UINT nObjCount)
	{
		UINT TotalSize = sizeof(stObjPool) + sizeof(stObjEntry) * (nObjCount - 1);
		stObjPool *pPool = (stObjPool*)_aligned_malloc(TotalSize, std::alignment_of<T>::value);

		if (pPool != nullptr)
		{
			pPool->Init(nObjCount, TotalSize);
			return pPool;
		}

		return nullptr;
	}

	UINT GetObjectIndex(T* pObj)
	{
		stObjEntry *pObjEntry = CONTAINING_RECORD(pObj, stObjEntry, pNext);
		return pObjEntry->pPool->GetIndex(pObjEntry);
	}
	UINT GetAllocatedPoolCount()
	{
		UINT nCount;
		for (nCount = 0; nCount < m_nPool; ++nCount)
			if (!m_pPoolArray[nCount])
				break;
		return nCount;
	}
	UINT State(UINT &nTotalObj, UINT &nFreeObj)
	{
		UINT nPoolCount;

		for (nFreeObj = 0, nPoolCount = 0; nPoolCount < m_nPool; ++nPoolCount)
			if (m_pPoolArray[nPoolCount])
				nFreeObj += m_pPoolArray[nPoolCount]->CountFreeObj();
			else
				break;

		nTotalObj = nPoolCount * m_nObjCountPerPool;

		return nPoolCount;
	}

	UINT CountFreeObj(UINT nPoolIndex)
	{
		if (nPoolIndex >= m_nPool || !m_pPoolArray[nPoolIndex])
			return 0;
		return m_pPoolArray[nPoolIndex]->CountFreeObj();
	}

	INLINE UINT32 GetMaxPoolCount() { return m_nPool; }
	INLINE void Info() 
	{
		printx("Object alignment: %d Bytes\n", std::alignment_of<T>::value);

		if (m_pPoolArray[0] != nullptr)
		{
			printx("Objects per pool: %d\n", m_pPoolArray[0]->m_nArraySize);
			printx("Space wasting rate: %f (%d / (%d * %d))\n", (DOUBLE)m_pPoolArray[0]->m_nPoolSize / (sizeof(T) * m_pPoolArray[0]->m_nArraySize),
			m_pPoolArray[0]->m_nPoolSize, sizeof(T), m_pPoolArray[0]->m_nArraySize);
		}
	}


protected:

	CCriticalSectionUTX m_cs;
	UINT32 m_nPool, m_nObjCountPerPool;
	stObjPool **m_pPoolArray;


};


struct stSimpleListNode
{
public:

	// Don't allocate by these methods that will involve constructor and destructor.
	void* operator new(size_t size) = delete;
	void operator delete(void *mem) = delete;

	INLINE static stSimpleListNode* alloc() { ASSERT(m_ObjectPool.GetMaxPoolCount() != 0); return m_ObjectPool.AllocObj(); }
	INLINE static void mfree(stSimpleListNode *ptr) { m_ObjectPool.FreeObj(ptr); }
	INLINE operator list_entry*() { return &list; }

	list_entry list;
	void* pData;


private:

	friend BOOL UTXLibraryInit();
	friend void UTXLibraryEnd();

	static TObjectPool<stSimpleListNode> m_ObjectPool;


};


BEGIN_NAMESPACE(UTX)


class CSimpleList
{
public:

	CSimpleList()
	:m_Count(0)
	{
		INIT_LIST_HEAD(&m_ListHead);
	}
	~CSimpleList()
	{
		if (m_Count != 0)
			clear();
	}

	INLINE BOOL push_front(void *ptr)
	{
		stSimpleListNode *pNewNode = stSimpleListNode::alloc();
		if (pNewNode != nullptr)
		{
			pNewNode->pData = ptr;
			list_add(*pNewNode, &m_ListHead);
			++m_Count;
			return TRUE;
		}
		return FALSE;
	}
	INLINE BOOL push_back(void *ptr)
	{
		stSimpleListNode *pNewNode = stSimpleListNode::alloc();
		if (pNewNode != nullptr)
		{
			pNewNode->pData = ptr;
			list_add_tail(*pNewNode, &m_ListHead);
			++m_Count;
			return TRUE;
		}
		return FALSE;
	}

	INLINE void* pop_front()
	{
		ASSERT(!LIST_IS_EMPTY(m_ListHead));

		stSimpleListNode *pHeadNode = (stSimpleListNode*)m_ListHead.next;
		void *ptrOut = pHeadNode->pData;
		list_remove(*pHeadNode);
		stSimpleListNode::mfree(pHeadNode);
		--m_Count;
		return ptrOut;
	}
	INLINE void* pop_back()
	{
		ASSERT(!LIST_IS_EMPTY(m_ListHead));

		stSimpleListNode *pTailNode = (stSimpleListNode*)m_ListHead.prev;
		void *ptrOut = pTailNode->pData;
		list_remove(*pTailNode);
		stSimpleListNode::mfree(pTailNode);
		--m_Count;
		return ptrOut;
	}

	INLINE void* front()
	{
		ASSERT(!LIST_IS_EMPTY(m_ListHead));
		return ((stSimpleListNode*)m_ListHead.next)->pData;
	}
	INLINE void* back()
	{
		ASSERT(!LIST_IS_EMPTY(m_ListHead));
		return ((stSimpleListNode*)m_ListHead.prev)->pData;
	}

	void clear()
	{
		list_entry *pEntry, *pNext;
		for (pEntry = m_ListHead.prev; IS_VALID_ENTRY(pEntry, m_ListHead); )
		{
			pNext = pEntry->next;
			stSimpleListNode::mfree((stSimpleListNode*)pEntry);
			pEntry = pNext;
		}
		INIT_LIST_HEAD(&m_ListHead);
		m_Count = 0;
	}


	INLINE void* GetAt(list_entry *entry) { return ((stSimpleListNode*)entry)->pData; }
	INLINE UINT32 size() const { return m_Count; }
	INLINE const list_head& GetListHead() const { return m_ListHead; }


protected:

	list_head m_ListHead;
	UINT32 m_Count;


};


END_NAMESPACE;


#endif


