/*
 * MmapVector.h
 *
 *  Created on: 17.12.2019
 *      Author: hartung
 */

#ifndef COMPLETESATANALYZER_MMAPVECTOR_H_
#define COMPLETESATANALYZER_MMAPVECTOR_H_

#include <cstdint>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cassert>
#include <iostream>

#include <sys/types.h>
#include <sys/mman.h>

namespace CompleteSatAnalyzer {

inline int closeC(int fd) {
	return close(fd);
}
inline int openC(const char *__file, int __oflag, int mode) {
	return open(__file, __oflag, mode);
}

class NotEnoughDiskSpaceException {
};

class MmapStorage {
public:

	MmapStorage(const MmapStorage&) = delete;
	MmapStorage& operator=(const MmapStorage&) = delete;
	typedef uint64_t size_type;

	MmapStorage() :
			fileHd(-1), sz(0), maxBytes(0), pageSize(getpagesize()), minFileSize(40*1024*1024), _data(
					nullptr) {

	}

	MmapStorage(MmapStorage&& in) :
			MmapStorage() {
		*this = std::move(in);
	}

	MmapStorage(bool const allocateMax, std::string const & filename,
			uint64_t const maxBytes) :
			MmapStorage() {
		open(allocateMax, filename, maxBytes);
	}
	MmapStorage& operator=(MmapStorage&& in) {
		std::swap(fileHd, in.fileHd);
		std::swap(sz, in.sz);
		std::swap(maxBytes, in.maxBytes);
		std::swap(_data, in._data);
		std::swap(filename, in.filename);
		assert(pageSize == in.pageSize);
		return *this;
	}

	void open(bool const allocateMax, std::string const & filename,
			uint64_t const maxBytes) {
		this->maxBytes = (maxBytes / pageSize) * pageSize;
		this->filename = filename;
		openFile();
		size_type const nBytes = std::min(this->maxBytes,
				(allocateMax) ? this->maxBytes : 1024ul * 1024ul * 50ul);
		resizeFile(nBytes);
		mapFile(nBytes);
	}

	void close() {
		if (_data != nullptr) {
			assert(fileHd != -1);
			unmapFile();
			closeC(fileHd);
			assert(fileExists(filename));
			if (remove(filename.c_str()) != 0)
				std::cerr << "c MmapStorage could not delete file" << std::endl;
		} else if (filename.size() > 0 && fileExists(filename)) {
			if (fileHd != -1)
				closeC(fileHd);
			if (remove(filename.c_str()) != 0)
				std::cerr << "c MmapStorage could not delete file" << std::endl;
		}
		_data = nullptr;
		fileHd = -1;
	}

	~MmapStorage() {
		close();
	}

	bool good() const {
		return _data != nullptr && fileHd != -1;
	}

	bool reserve(size_type const n) {
		bool res = true;
		if (n != 0 && n > capacity()) {
			size_type nBytes = n + sizeof(size_type), cap = capacity();
			nBytes += pageSize - nBytes % pageSize;
			nBytes = std::min(std::max(nBytes,minFileSize), maxBytes);
			assert(nBytes % pageSize == 0);

			if (_data != nullptr)
				unmapFile();
			res = resizeFile(nBytes);
			if (!res)
				nBytes = cap;
			if (fileHd != -1 && nBytes > 0)
				mapFile(nBytes); // still map the file, even when an increase failed
		}
		return res;
	}

	static size_type npos() {
		return std::numeric_limits<size_type>::max();
	}

	size_type alloc(size_type const nBytes) {
		if (sz + nBytes > capacity()) {
			if (sz + nBytes > nMaxBytes())
				return npos();
			uint64_t const cap = capacity();
			assert(maxBytes > cap);
			uint64_t const toAdd = std::max(((cap >> 1) + (cap >> 3) + 2) & ~1,
							nBytes); // from minisat

			if (!reserve(cap + toAdd) || nBytes + sz > capacity())
				return npos();
		}
		size_type const res = sz;
		sz += nBytes;
		return res;
	}

	size_type size() const {
		return sz;
	}

	char const * data() const {
		return _data;
	}

	char * data() {
		return _data;
	}

	size_type capacity() const {
		return (_data != nullptr) ?
				*reinterpret_cast<size_type const*>(rawData())
						- sizeof(size_type) :
				0;
	}

	size_type nMaxBytes() const {
		return maxBytes - sizeof(size_type);
	}

private:
	int fileHd;
	size_type sz;
	uint64_t maxBytes;
	uint64_t const pageSize;
	uint64_t minFileSize;
	char * _data;
	std::string filename;

	uint64_t getCapacityFromFile() {
		// read capacity from the first bytes of the file:
		assert(fileHd > -1);
		void * ptr = mmap(NULL, 8, PROT_WRITE | PROT_READ, MAP_SHARED, fileHd,
				0);
		if (ptr == MAP_FAILED) {
			std::cerr << "c MmapStorage: Could not map file\n";
			throw std::bad_alloc();
		}
		uint64_t cap = *reinterpret_cast<size_type const*>(ptr);
		if (munmap(ptr, 8) != 0) {
			std::cerr << "c MmapStorage: Could not map file\n";
			throw std::bad_alloc();
		}
		return cap;
	}

	void mapFile(size_type const cap) {
		assert(cap > 0);
		char * ptr = reinterpret_cast<char*>(mmap(NULL, cap,
		PROT_WRITE | PROT_READ,
		MAP_SHARED, fileHd, 0));
		if ((void*) ptr == MAP_FAILED) {
			std::cerr << "c MmapStorage: Could not map file\n";
			throw std::bad_alloc();
		}
		// save cap in the first bytes:
		*reinterpret_cast<size_type*>(ptr) = cap;
		// shift:
		_data = ptr + sizeof(size_type);
		assert(cap - sizeof(size_type) == capacity());
	}

	void unmapFile() {
		size_type cap = capacity();
		if (munmap(rawData(), cap) != 0) {
			std::cerr << "c MmapStorage: Unmap went wrong\n";
			throw std::bad_alloc();
		}

		_data = nullptr;
	}

	void openFile() {
		assert(fileHd == -1);
		fileHd = openC(filename.c_str(), O_RDWR | O_CREAT, S_IRWXU);
		if (fileHd < 0) {
			std::cerr << "c MmapStorage: Could not open file '"
					<< filename.c_str() << "'" << std::endl;
			throw std::bad_alloc();
		}
	}

	bool resizeFile(const size_type & n) {
		if (fileHd == -1)
			openFile();
		if (n > maxBytes || ftruncate(fileHd, n) < 0) {
			std::cerr << "c MmapStorage: Could not resize file\n";
			return false;
		}
		return true;
	}

	bool fileExists(std::string const & filename) {
		if (access(filename.c_str(), F_OK) == -1)
			return false;

		return true;
	}

	char const * rawData() const {
		assert(_data != nullptr);
		return reinterpret_cast<char const*>(_data) - sizeof(size_type);
	}

	char * rawData() {
		assert(_data != nullptr);
		return reinterpret_cast<char*>(_data) - sizeof(size_type);
	}

};
}

#endif /* COMPLETESATANALYZER_MMAPVECTOR_H_ */
