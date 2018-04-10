#pragma once
#include <string>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>


class DataItem
{
public:
	std::string key;
	uint64_t rank;
	DataItem(std::string k, uint64_t r) :key(k), rank(r) {}
	bool operator<(const DataItem& e)const { return rank < e.rank; }
};

typedef boost::multi_index::multi_index_container
<
	DataItem,
	boost::multi_index::indexed_by
	<
	boost::multi_index::hashed_unique<boost::multi_index::member<DataItem, std::string, &DataItem::key>>,

	boost::multi_index::ordered_unique<boost::multi_index::member<DataItem,uint64_t, &DataItem::rank>>
	>
> DataItemContainer;


class  EvictionStrategy
{
public:

	virtual ~EvictionStrategy() {}

	virtual std::string nextEviction()=0;

	virtual void addKey(std::string)=0;

	virtual void refreshKey(std::string) = 0;

	virtual void deleteKey(std::string)=0;
};

class OldestInsertionEviction :public EvictionStrategy
{
public:
	 OldestInsertionEviction();

	 std::string nextEviction() ;

	 void addKey(std::string) ;

	 void refreshKey(std::string);

	 void deleteKey(std::string);
private:
	uint64_t getNextRank() { return currentRank++; }//NOTE not thread safe
	uint64_t currentRank;
	DataItemContainer dataItems_;
};