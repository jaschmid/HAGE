#include <array>
#include <unordered_map>

template<class _Key,_Key _Size,class _Item> class FixedSizeKeyStorage
{
private:
	typedef typename std::unordered_map<_Item,_Key,std::tr1::hash<_Item>,std::equal_to<_Item>,HAGE::global_allocator<std::pair<_Item,_Key>>> ItemToKeyMapType;
	ItemToKeyMapType				m_ItemToKeyMap;
	typedef typename std::array<_Item,_Size> ItemStorageType;
	ItemStorageType					m_ItemStorage;
	_Key							m_NextFreeItem;

public:
	_Key GetKey(const _Item& item)
	{
		typename ItemToKeyMapType::iterator found(m_ItemToKeyMap.find(item));
		if(found==m_ItemToKeyMap.end())
		{
			assert(m_NextFreeItem<_Size);
			m_ItemToKeyMap.insert(std::pair<_Item,_Key>(item,m_NextFreeItem));
			m_ItemStorage[m_NextFreeItem]=item;
			_Key result = m_NextFreeItem;
			++m_NextFreeItem;
			return result;
		}
		assert(found!=m_ItemToKeyMap.end());
		return found->second;
	}
	const _Item& GetItem(const _Key& key) const
	{
		assert(key<m_NextFreeItem);
		return m_ItemStorage[key];
	}
};
