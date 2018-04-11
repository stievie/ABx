#pragma once
#include <unordered_map>
#include <vector>
#include "EvictionStrategy.h"

class StorageProvider
{
public:
	StorageProvider(size_t maxSize);
	void Save(const std::vector<uint8_t>& key,
		std::shared_ptr<std::vector<uint8_t>>  data);
	std::shared_ptr<std::vector<uint8_t>> Get(const std::vector<uint8_t>& key);
	bool Remove(const std::vector<uint8_t>& key);
    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id);
    static std::vector<uint8_t> EncodeKey(const std::string& table, uint32_t id);
private:
	bool EnoughSpace(size_t size);
	void CreateSpace(size_t size);
	bool RemoveData(const std::string& key);
    std::shared_ptr<std::vector<uint8_t>> LoadData(const std::vector<uint8_t>& key);

	std::unordered_map<std::string, std::shared_ptr<std::vector<uint8_t>>> cache_;
	size_t currentSize_;
	size_t maxSize_;
	std::shared_ptr<EvictionStrategy> evictor_;
};

