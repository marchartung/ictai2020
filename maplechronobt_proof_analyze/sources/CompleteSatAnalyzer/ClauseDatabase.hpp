/*
 * ClauseDataBase.h
 *
 *  Created on: 17.12.2019
 *      Author: hartung
 */

#ifndef COMPLETESATANALYZER_CLAUSEDATABASE_H_
#define COMPLETESATANALYZER_CLAUSEDATABASE_H_

#include <string>
#include <sstream>
#include <cstdint>
#include <algorithm>

#include "MmapVector.h"
#include "ClauseUsageTypes.h"

namespace CompleteSatAnalyzer {

enum class ClauseOrigin {
	INITIAL = 0,
	UIP1 = 1,
	MULTI_UIP = 2,
	MULTI_CONFLICT = 3,
	UPPER_MIN_CUT = 4,
	LF1 = 5,
	PREPROCESSING = 6,
	VIVIFICATION = 7,
	REDUNDANT = 8,
	UNDEF = 9 // insert new group before undef and change numbers
};

inline constexpr uint8_t groupToInt(ClauseOrigin const o) {

	return static_cast<unsigned>(o);
}

inline ClauseOrigin intToGroup(uint8_t const id) {
	return static_cast<ClauseOrigin>(id);
}

inline std::string substr(std::string const & str,
		std::string::size_type const sP, std::string::size_type const eP) {
	assert(sP < str.size());
	return str.substr(sP,
			(eP != std::string::npos) ? eP - sP : std::string::npos);
}

inline std::string groupToString(ClauseOrigin const o) {
	switch (o) {
	case ClauseOrigin::INITIAL:
		return "Initial";
	case ClauseOrigin::UIP1:
		return "1UIP";
	case ClauseOrigin::MULTI_UIP:
		return "Multi UIP";
	case ClauseOrigin::MULTI_CONFLICT:
		return "Multi Conflict";
	case ClauseOrigin::UPPER_MIN_CUT:
		return "Upper Min Cut";
	case ClauseOrigin::LF1:
		return "LF1";
	case ClauseOrigin::PREPROCESSING:
		return "Preprocessor";
	case ClauseOrigin::VIVIFICATION:
		return "Vivification";
	case ClauseOrigin::REDUNDANT:
		return "Proof Linking Units";
	default:
		return "Undefined";
	}

}

inline std::string groupIdToString(unsigned const id) {
	return groupToString(intToGroup(id));
}

typedef uint64_t Cid;

template<typename IntType, unsigned nBit>
constexpr IntType trimToMaxBit(IntType const & in) {
	constexpr const IntType maxI = std::numeric_limits<IntType>::max()
			>> sizeof(IntType) * 8 - nBit;
	static_assert((((IntType)1) << nBit)-1 == maxI, "Something is wrong with the bit trimming");
	return (maxI > in) ? in : maxI;
}

#define CA_CID_INTERNAL_GROUPID_BITS 4
#define CA_CID_INTERNAL_LBD_BITS 4

struct Cid_Internal {

	static unsigned const maxTrackedLbd = 9;
	static unsigned const maxTrackedGroup = groupToInt(ClauseOrigin::UNDEF);
	static_assert(maxTrackedLbd == trimToMaxBit<uint64_t,CA_CID_INTERNAL_LBD_BITS>(maxTrackedLbd), "Bit width cannot express maxTrackedLbd");

	Cid_Internal() = delete;
	Cid_Internal & operator=(Cid_Internal const & in) {
		// hacky:
		static_assert(sizeof(*this) == 8, "Cid size changed");
		toUint() = in.toUint();
		return *this;
	}

	Cid_Internal & operator=(Cid_Internal && in) {
		// hacky:
		toUint() = in.toUint();
		return *this;
	}

	Cid_Internal(Cid_Internal const & in) {
		*this = in;
	}
	Cid_Internal(Cid_Internal && in) {
		*this = std::move(in);
	}

	static uint64_t maxBytes() {
		return cast(undef()).id;
	}

	uint64_t fId :2;
	uint64_t groupId :CA_CID_INTERNAL_GROUPID_BITS;
	uint64_t lbdId :CA_CID_INTERNAL_LBD_BITS;
	uint64_t id :54;

	static Cid_Internal const & cast(uint64_t const & ui) {
		return *reinterpret_cast<Cid_Internal const *>(&ui);
	}

	static Cid_Internal & cast(uint64_t & ui) {
		return *reinterpret_cast<Cid_Internal *>(&ui);
	}

	uint64_t const & toUint() const {
		return *reinterpret_cast<uint64_t const *>(this);
	}

	uint64_t & toUint() {
		return *reinterpret_cast<uint64_t *>(this);
	}

	static uint64_t create(unsigned const fId, ClauseOrigin const o,
			unsigned const lbd, uint64_t const id) {
		return Cid_Internal(fId, o, lbd, id).toUint();
	}

	static uint64_t undef() {
		return std::numeric_limits<uint64_t>::max();
	}

private:
	Cid_Internal(unsigned const fId, ClauseOrigin const o, unsigned const lbd,
			uint64_t const id) :
			fId(fId), groupId(groupToInt(o)), lbdId(
					(lbd < maxTrackedLbd) ? lbd : maxTrackedLbd), id(id) {
		assert(fId < 4);
	}
};
static_assert(sizeof(Cid) == sizeof(Cid_Internal), "ClauseDatabase: Cid and Cid_Internal are different in size");

static Cid const Cid_Undef = Cid_Internal::undef();

struct Clause {

private:

	struct Header {
		uint16_t usedInProof :1;
		uint16_t deleteClause :1;
		uint16_t groupId :14;
		uint16_t lbd;
		uint32_t csz;
		uint32_t rsz;
		uint32_t level;
		uint32_t nOpen;
		uint32_t proofId;
	} h;
	static_assert(sizeof(Header) % 8 == 0,"clause should be at least 64bit aligned");
	uint64_t data[1];

public:
	typedef uint32_t size_type;

	uint32_t getLbd() const {
		return h.lbd;
	}

	inline size_type nLits() const {
		return h.csz;
	}

	uint32_t getGroupInt() const {
		return h.groupId;
	}

	ClauseOrigin getGroup() const {
		return intToGroup(getGroupInt());
	}

	void markRemoved() {
		assert(!isDeleteClause());
		setDeleteClause(true);
	}

	void setGroup(ClauseOrigin const o) {
		h.groupId = groupToInt(o);
	}

	void setProofId(uint32_t const id) {
		h.proofId = id;
	}

	uint32_t getProofId() const {
		return h.proofId;
	}

	void setNumLits(size_type const n) {
		h.csz = n;
	}

	void setOpen(uint32_t const v) {
		h.nOpen = v;
	}

	void setOpenResolved(uint32_t const nOpenResolved) {
		h.nOpen = nOpenResolved;
	}

	bool isOpenResolved() const {
		return h.nOpen > 0;
	}

	bool isAsserting() const {
		assert(h.nOpen > 0);
		return h.nOpen == 1;
	}

	void setNumResolvents(size_type const n) {
		h.rsz = n;
	}

	int size() const {
		return h.csz;
	}

	Lit const & operator[](const int idx) const {
		return getLit(idx);
	}

	size_type nResolvents() const {
		return h.rsz;
	}

	inline Lit const * lbegin() const {
		return lits();
	}

	inline Lit const * lend() const {
		return lits() + h.csz;
	}

	inline Lit const& getLit(size_type const idx) const {
		assert(idx < h.csz);
		return lits()[idx];
	}

	inline Lit& getLit(size_type const idx) {
		assert(idx < h.csz);
		return lits()[idx];
	}

	inline Cid const * rbegin() const {
		return resolvents();
	}

	inline Cid const * rend() const {
		return resolvents() + h.rsz;
	}

	Cid const& getResolvent(size_type const &idx) const {
		assert(idx < h.rsz);
		return resolvents()[idx];
	}

	Cid& getResolvent(size_type const &idx) {
		assert(idx < h.rsz);
		assert(
				reinterpret_cast<char const*>(this) + nBytes(h.csz, h.rsz)
						>= reinterpret_cast<char const*>(resolvents() + h.rsz));
		return resolvents()[idx];
	}

	void setUsedInProof(bool const b) {
		h.usedInProof = b;
	}

	void setLbd(unsigned const l) {
		h.lbd = trimToMaxBit<uint32_t, 16>(l);
	}

	void setLevel(uint64_t const &lvl) {
		h.level = lvl;
	}

	void setDeleteClause(bool const b) {
		h.deleteClause = b;
	}

	bool isDeleteClause() const {
		return h.deleteClause;
	}

	uint64_t getLevel() const {
		return h.level;
	}

	bool isUsedInProof() const {
		return h.usedInProof;
	}

	static uint64_t make64Bit(uint64_t const cid) {
		return (cid % 8 == 0) ? cid : (cid - (cid % 8) + 8);
	}

	static uint64_t nBytes(size_type const nLits, size_type const nResolvents) {
		static const uint64_t numBytesAlreadyInClause = sizeof(Clause)
				- sizeof(Header);
		static_assert(sizeof(Clause) == sizeof(Header)+sizeof(uint64_t),"Clause header and Clause size do not add up");
		uint64_t numBytesNeeded = nLits * sizeof(Lit)
				+ nResolvents * sizeof(Cid);
		return sizeof(Clause)
				+ ((numBytesNeeded > numBytesAlreadyInClause) ?
						make64Bit(numBytesNeeded - numBytesAlreadyInClause) : 0);
	}

private:
	// No Constructor due to in memory allocation
	Clause() = delete;
	Clause(Clause const&) = delete;
	Clause(Clause&&) = delete;
	Clause& operator=(Clause const&) = delete;
	Clause& operator=(Clause&&) = delete;

	inline Lit const* lits() const {
		return reinterpret_cast<Lit const*>(data);
	}

	inline Lit* lits() {
		return reinterpret_cast<Lit*>(data);
	}

	inline Cid const* resolvents() const {
		return reinterpret_cast<Cid const*>(lits() + h.csz);
	}

	inline Cid* resolvents() {
		return reinterpret_cast<Cid*>(lits() + h.csz);
	}
};

class ClauseView {
public:
	ClauseView(Clause const &c) :
			c(c) {
	}

	int size() const {
		return c.nLits();
	}

	Lit const& operator[](int const idx) const {
		return c.getLit(idx);
	}
private:
	Clause const &c;
};

class ClauseDatabase {
public:

	typedef MmapStorage::size_type size_type;

	ClauseDatabase(std::string const & configString) :
			curDataId(0), nClauses(0) {
		if(configString.size() > 0)
			createFromConfigString(configString);
	}

	~ClauseDatabase() {
	}

	void createFromConfigString(std::string const & configString) {
		// string : "/path/to/file1,4553:/path/to/file2:1000000" numbers in mb
		std::string::size_type pos = 0, curEndPos, commaPos;
		while (pos != std::string::npos) {
			curEndPos = configString.find(':', pos + 1);
			commaPos = substr(configString, pos, curEndPos).find(',');
			if(commaPos != std::string::npos)
				commaPos += pos;
			assert(commaPos == std::string::npos || commaPos > pos);
			std::string const filename = substr(configString, pos, std::min(commaPos,curEndPos));
			uint64_t nbytes = Cid_Internal::maxBytes();
			if (commaPos != std::string::npos && commaPos < curEndPos) {
				assert(commaPos + 1 < curEndPos);
				nbytes = std::stoul(
						substr(configString, commaPos + 1, curEndPos)) * 1000ul
						* 1000ul;
			}
			storages.emplace_back(false, filename,
					nbytes);
			pos = (curEndPos != std::string::npos) ?
					curEndPos + 1 : std::string::npos;
		}
	}

	uint64_t getUsedBytes() const {
		uint64_t sum = 0;
		for (auto const & store : storages) {
			sum += store.size();
		}
		return sum;
	}

	Cid_Internal allocateStorage(size_type const nBytes) {
		size_type res = MmapStorage::npos();
		do {
			res = storages[curDataId].alloc(nBytes);
			curDataId += res == MmapStorage::npos();
		} while (res == MmapStorage::npos() && curDataId < storages.size());

		if (res == MmapStorage::npos()) {
			std::cerr << "c MmapStorage: Could not resize file\n";
			throw NotEnoughDiskSpaceException();
		}
		return Cid_Internal::cast(
				Cid_Internal::create(curDataId, ClauseOrigin::UNDEF, 0, res));
	}

	void addDeleteClause(const uint64_t conflictNum, Cid const cid) {
		++nClauses;
		size_type nBytes = Clause::nBytes(0, 1);
		Cid_Internal const pos = allocateStorage(nBytes);
		Clause &newClause = getClause(pos);
		newClause.setUsedInProof(false);
		newClause.setDeleteClause(true);
		newClause.setGroup(intToGroup(getGroupIntFast(cid)));
		newClause.setNumLits(0);
		newClause.setNumResolvents(1);
		newClause.setOpen(0);
		newClause.setProofId(std::numeric_limits<uint32_t>::max());

		newClause.getResolvent(0) = cid;
	}

	template<typename ClauseType, typename CidVec>
	Cid addClause(const uint64_t conflictNum, ClauseType const &c,
			CidVec const &resolutions, unsigned const lbd, unsigned const nOpen,
			ClauseOrigin const origin) {
		++nClauses;
		size_type nBytes = Clause::nBytes(c.size(), resolutions.size());
		Cid_Internal const pos = allocateStorage(nBytes);
		Clause &newClause = getClause(pos);

		newClause.setUsedInProof(false);
		newClause.setDeleteClause(false);
		newClause.setGroup(origin);
		newClause.setLbd(lbd);
		newClause.setNumLits(c.size());
		newClause.setNumResolvents(resolutions.size());
		newClause.setOpen(nOpen);
		newClause.setProofId(std::numeric_limits<uint32_t>::max());

		for (Clause::size_type i = 0; i < newClause.nLits(); ++i) {
			newClause.getLit(i) = c[i];
		}
		for (Clause::size_type i = 0; i < newClause.nResolvents(); ++i)
			newClause.getResolvent(i) = resolutions[i];

		return Cid_Internal::create(pos.fId, origin, lbd, pos.id);
	}

	void setUsedInProof(Cid const cid, bool const b) {
		(*this)[cid].setUsedInProof(b);
	}

	inline Clause const& operator[](Cid const & cid) const {
		return getClause(Cid_Internal::cast(cid));
	}

	inline Clause& operator[](Cid const cid) {
		return getClause(Cid_Internal::cast(cid));
	}

	Cid end() const {
		return Cid_Undef;
	}

	Cid begin() const {
		return (storages[0].size() > 0) ?
				Cid_Internal::create(0, getPos(0, 0).getGroup(),
						getPos(0, 0).getLbd(), 0) :
				end();
	}

	void next(Cid &cur) const {
		Clause const &co = operator[](cur);
		uint64_t const newPos = Cid_Internal::cast(cur).id
				+ Clause::nBytes(co.nLits(), co.nResolvents());
		uint64_t const storeId = Cid_Internal::cast(cur).fId;

		if (newPos < storages[storeId].size()) { // clause in same storage
			Clause const & nc = getPos(storeId, newPos);
			cur = Cid_Internal::create(storeId, nc.getGroup(), nc.getLbd(),
					newPos);
		} else {
			if (Cid_Internal::cast(cur).fId + 1 < storages.size()
					&& storages[Cid_Internal::cast(cur).fId + 1].size() > 0) { // clause in next storage
				Clause const & nc = getPos(Cid_Internal::cast(cur).fId, newPos);
				cur = Cid_Internal::create(storeId + 1, nc.getGroup(),
						nc.getLbd(), 0);
			} else
				// end of buffer
				cur = end();
		}
	}

	unsigned getGroupIntFast(Cid const & cid) const {
		return Cid_Internal::cast(cid).groupId;
	}

	unsigned getGroupLbdFast(Cid const & cid) const {
		return Cid_Internal::cast(cid).lbdId;
	}

	bool good() const {
		return storages.size() > curDataId && storages[curDataId].good();
	}

private:
	size_type curDataId;
	uint64_t nClauses;
	std::vector<MmapStorage> storages;

	Clause const& getClause(Cid_Internal const & cid) const {
		return getPos(cid.fId, cid.id);
	}

	Clause& getClause(Cid_Internal const & cid) {
		return getPos(cid.fId, cid.id);
	}

	Clause const & getPos(uint64_t const storeId, uint64_t const pos) const {
		assert(pos >> 54 == 0);
		return *reinterpret_cast<Clause const*>(storages[storeId].data() + pos);
	}

	Clause & getPos(uint64_t const storeId, uint64_t const pos) {
		assert(pos >> 56 == 0);
		return *reinterpret_cast<Clause*>(storages[storeId].data() + pos);
	}

};
}

#endif /* COMPLETESATANALYZER_CLAUSEDATABASE_H_ */
