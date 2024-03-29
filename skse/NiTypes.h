#pragma once

#include "skse\Utilities.h"

// 4
template <class T>
class NiPointer
{
public:
	T	* m_pObject;	// 00

	inline NiPointer(T* pObject = 0)
	{
		m_pObject = pObject;
		if(m_pObject) m_pObject->IncRef();
	}

	inline NiPointer(const T& rhs)
	{
		if(rhs.m_pObject) rhs.m_pObject->IncRef();
		m_pObject = rhs.m_pObject;
	}

	inline NiPointer(T&& rhs)
	{
		m_pObject = rhs.m_pObject;
		rhs.m_pObject = 0;
	}

	inline ~NiPointer()
	{
		if(m_pObject) m_pObject->DecRef();
	}

	inline operator T *() const
	{
		return m_pObject;
	}

	inline T & operator*() const
	{
		return *m_pObject;
	}

	inline T * operator->() const
	{
		return m_pObject;
	}

	inline NiPointer <T> & operator=(const NiPointer & rhs)
	{
		if(m_pObject != rhs.m_pObject)
		{
			if(rhs.m_pObject) rhs.m_pObject->IncRef();
			if(m_pObject) m_pObject->DecRef();

			m_pObject = rhs.m_pObject;
		}

		return *this;
	}
	
	inline NiPointer <T> & operator=(NiPointer && rhs)
	{
		if(m_pObject != rhs.m_pObject)
			std::swap(m_pObject, rhs.m_pObject);

		return *this;
	}

	inline NiPointer <T> & operator=(T * rhs)
	{
		if(m_pObject != rhs)
		{
			if(rhs) rhs->IncRef();
			if(m_pObject) m_pObject->DecRef();

			m_pObject = rhs;
		}

		return *this;
	}

	inline bool operator==(T * pObject) const
	{
		return m_pObject == pObject;
	}

	inline bool operator!=(T * pObject) const
	{
		return m_pObject != pObject;
	}

	inline bool operator==(const NiPointer & ptr) const
	{
		return m_pObject == ptr.m_pObject;
	}

	inline bool operator!=(const NiPointer & ptr) const
	{
		return m_pObject != ptr.m_pObject;
	}
	
	MEMBER_FN_PREFIX(NiPointer);
	DEFINE_MEMBER_FN(assign, void, 0x004B7A20, T*); // not sure
};

template <class T>
struct std::hash<NiPointer<T>>
{
	inline std::size_t operator()(const NiPointer<T>& a)const{ return std::hash<void*>()(a); }
};

#define MAKE_NI_POINTER(x)	class x; typedef NiPointer <x> x##Ptr

template <class T_to, class T_from>
T_to * niptr_cast(const T_from & src)
{
	return static_cast <T_to *>(src.m_pObject);
}

// 10
class NiQuaternion
{
public:
	// w is first

	float	m_fW;	// 0
	float	m_fX;	// 4
	float	m_fY;	// 8
	float	m_fZ;	// C
};

// C
class NiPoint3
{
public:
	float	x;	// 0
	float	y;	// 4
	float	z;	// 8
};

// 0C
class NiColor
{
public:
	float	r;	// 0
	float	g;	// 4
	float	b;	// 8
};

// 10
class NiColorA
{
public:
	float	r;	// 0
	float	g;	// 4
	float	b;	// 8
	float	a;	// C
};

// 24
class NiMatrix33
{
public:
	float	data[9];
};

STATIC_ASSERT(sizeof(NiMatrix33) == 0x24);

// 34
class NiTransform
{
public:
	NiMatrix33	rot;	// 00
	NiPoint3	pos;	// 24
	float		scale;	// 30
};

STATIC_ASSERT(sizeof(NiTransform) == 0x34);

// 10
class NiBound
{
public:
	NiPoint3	pos;
	float		radius;
};

STATIC_ASSERT(sizeof(NiBound) == 0x10);

// 10
// derives from NiTMapBase, we don't bother
template <typename T_key, typename T_data>
class NiTMap
{
public:
	virtual ~NiTMap();

	struct NiTMapItem
	{
		NiTMapItem	* next;
		T_key		key;
		T_data		data;
	};

	T_data	Get(T_key key)
	{
		UInt32	bucket = GetBucket(key);

		for(NiTMapItem * iter = buckets[bucket]; iter; iter = iter->next)
		{
			if(Compare(iter->key, key))
			{
				return iter->data;
			}
		}

		return T_data();
	}

	virtual UInt32	GetBucket(T_key key);					// return hash % numBuckets;
	virtual bool	Compare(T_key lhs, T_key rhs);			// return lhs == rhs;
	virtual void	FillItem(NiTMapItem * item, T_key key, T_data data);
															// item->key = key; item->data = data;
	virtual void	Fn_04(UInt32 arg);	// nop
	virtual NiTMapItem *	AllocItem(void);				// return new NiTMapItem;
	virtual void	FreeItem(NiTMapItem * item);			// item->data = 0; delete item;

//	void		** _vtbl;	// 00
	UInt32		numBuckets;	// 04
	NiTMapItem	** buckets;	// 08
	UInt32		numEntries;	// 0C
};

// 10
template <typename T_key, typename T_data>
class NiTPointerMap : NiTMap <T_key, T_data>
{
public:
};

// 10
template <typename T>
class NiTArray
{
public:
	NiTArray();
	virtual ~NiTArray();

	// sparse array, can have NULL entries that should be skipped
	// iterate from 0 to m_emptyRunStart - 1

//	void	** _vtbl;			// 00
	T		* m_data;			// 04
	UInt16	m_arrayBufLen;		// 08 - max elements storable in m_data
	UInt16	m_emptyRunStart;	// 0A - index of beginning of empty slot run
	UInt16	m_size;				// 0C - number of filled slots
	UInt16	m_growSize;			// 0E - number of slots to grow m_data by

	inline T& operator[](int idx){ return m_data[idx]; }
	inline const T& operator[](int idx)const{ return m_data[idx]; }
};

STATIC_ASSERT(sizeof(NiTArray <void *>) == 0x10);

// 18
template <typename T>
class NiTLargeArray
{
public:
	NiTLargeArray();
	virtual ~NiTLargeArray();

	// fast, doesn't search for empty slots
	void NiTLargeArray::Append(T item)
	{
		if(m_emptyRunStart == m_arrayBufLen)
		{
			// need to expand the array
			Resize(m_arrayBufLen + 1);
		}

		m_data[m_emptyRunStart] = item;
		m_emptyRunStart++;
		m_size++;
	}

	void NiTLargeArray::Resize(UInt32 size)
	{
		// not reclaiming memory yet
		if(size <= m_size) return;

		ASSERT(m_growSize);

		// obey min grow size
		UInt32	growSize = size - m_size;
		if(growSize < m_growSize)
			growSize = m_growSize;

		size = m_arrayBufLen + growSize;

		// create new array
		T	* newData = (T *)FormHeap_Allocate(sizeof(T) * size);

		for(UInt32 i = 0; i < size; i++)
		{
			new (&newData[i]) T;
			newData[i] = 0;
		}

		// copy over data, compacting as we go
		UInt32	iter = 0;

		for(UInt32 i = 0; i < m_emptyRunStart; i++)
		{
			if(m_data[i])
			{
				newData[iter] = m_data[i];
				iter++;
			}
		}

		// update pointers
		T		* oldData = m_data;
		UInt32	oldDataLen = m_emptyRunStart;

		m_data = newData;
		m_arrayBufLen = size;
		m_emptyRunStart = m_size;

		// delete old array
		if(oldData)
		{
			for(UInt32 i = 0; i < oldDataLen; i++)
				if(oldData[i])
					oldData[i].~T();

			FormHeap_Free(oldData);
		}
	}

//	void	** _vtbl;			// 00
	T		* m_data;			// 04
	UInt32	m_arrayBufLen;		// 08 - max elements storable in m_data
	UInt32	m_emptyRunStart;	// 0C - index of beginning of empty slot run
	UInt32	m_size;				// 10 - number of filled slots
	UInt32	m_growSize;			// 14 - number of slots to grow m_data by
};
