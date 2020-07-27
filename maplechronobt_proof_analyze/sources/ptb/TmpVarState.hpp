/*
 * TmpVarState.h
 *
 *  Created on: 08.06.2020
 *      Author: hartung
 */

#ifndef SOURCES_COMPLETESATANALYZER_TMPVARSTATE_H_
#define SOURCES_COMPLETESATANALYZER_TMPVARSTATE_H_

#include "ClauseUsageTypes.hpp"

namespace PTB {

class TmpVarStates {
public:
	TmpVarStates() :
			curTmpStore(0) {

	}

	void resize(int const n) {
		seen.resize(n);
		varDepend.resize(n, -1);
	}

	int size() const
	{
		return seen.size();
	}

	bool isSeen(int const v) const {
		return seen[v];
	}

	void setSeen(int const v) {
		assert(!seen[v]);
		seen[v] = true;
		toClear.push_back(v);
	}

	void setMaxDeps(int const n) {
		for (unsigned i = 0; i < nClDeps.size(); ++i)
			assert(nClDeps[i] == 0);
		nClDeps.resize(n, 0);
		curTmpStore = 0;
	}

	void addDep(int const v, unsigned const cid) {

		if (varDepend[v] == -1) {
			if (tmpCidStorage.size() == curTmpStore)
				tmpCidStorage.push_back(decltype(tmpCidStorage)::value_type());
			tmpCidStorage[curTmpStore].clear();
			varDepend[v] = curTmpStore++;
		}
		tmpCidStorage[varDepend[v]].push_back(cid);
		++nClDeps[cid];
	}

	void clearDep(unsigned const cid)
	{
		assert(nClDeps[cid] < 2);
		nClDeps[cid] = 0;
	}

	unsigned removeDep(unsigned const cid) {
		assert(nClDeps[cid] > 0);
		return --nClDeps[cid];
	}

	std::vector<unsigned> const & varDeps(int const v) {
		assert(varDepend[v] != -1);
		return tmpCidStorage[varDepend[v]];
	}

	bool hasDeps(int const v)
	{
		return varDepend[v] != -1;
	}

	void clear() {
		for (unsigned i = 0; i < toClear.size(); ++i) {
			seen[toClear[i]] = 0;
			varDepend[toClear[i]] = -1;
		}
		for(unsigned i=0;i<nClDeps.size();++i)
			assert(nClDeps[i] == 0);
		toClear.clear();
		curTmpStore = 0;
	}

private:
	unsigned curTmpStore;
	std::vector<bool> seen;
	std::vector<int> varDepend;
	std::vector<int> nClDeps;
	std::vector<std::vector<unsigned>> tmpCidStorage;
	std::vector<int> toClear;

};

}

#endif /* SOURCES_COMPLETESATANALYZER_TMPVARSTATE_H_ */
