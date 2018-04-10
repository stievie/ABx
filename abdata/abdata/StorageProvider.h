#pragma once
#include <unordered_map>
#include <vector>
#include "EvictionStrategy.h"


class StorageProvider
{
public:
	StorageProvider(uint32_t maxSize);

	void save(const std::vector<uint8_t>& key,
		std::shared_ptr<std::vector<uint8_t>>  data);

	std::shared_ptr<std::vector<uint8_t>> get(const std::vector<uint8_t>& key);

	int remove(const std::vector<uint8_t>& key);

private:
	bool enoughSpace(uint32_t size);

	void createSpace(uint32_t size);

	bool removeData(const std::string& key);

	std::unordered_map<std::string, std::shared_ptr<std::vector<uint8_t>>> cache_;
	uint32_t currentSize_;
	uint32_t maxSize_;
	std::shared_ptr<EvictionStrategy> evictor_;
};

