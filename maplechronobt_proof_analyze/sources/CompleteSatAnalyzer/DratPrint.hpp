/*
 * DratPrint.h
 *
 *  Created on: 10.04.2019
 *      Author: hartung
 */

#ifndef MTL_DRATPRINT_H_
#define MTL_DRATPRINT_H_

#include "ClauseUsageTypes.h"
#include <cstdio>
#include <string>
#include <iostream>

namespace CompleteSatAnalyzer {

class DratPrint {
public:
	DratPrint(const bool shouldPrint, const std::string & proofFileName) :
			proof((shouldPrint) ? fopen(proofFileName.c_str(), "wb") : NULL) {
		if (proof != NULL)
			flockfile(proof);
	}

	~DratPrint() {
		if (isActive()) {
			assert(proof != NULL);
			funlockfile(proof);
			fclose(proof);
			proof = NULL;
		}
	}

	constexpr bool isActive() const {
		return proof != NULL;
	}

	template<typename VecType>
	void addClause(const VecType & c) {
		addClause('a', c);
	}

	template<typename VecType>
	void addClauseExcludeLit(const VecType & c, const Lit l) {
		if (proof != NULL) {
			write('a');
			for (int i = 0; i < c.size(); ++i)
				if (c[i] != l)
					writeLit(c[i]);
			write(0);
		}
	}

	void addEmptyClause() {
		addClause('a', vec<Lit>());
	}
//-18168 -11304
	template<typename VecType>
	void removeClause(const VecType & c) {
		//addClause('d', c); FIXME
	}
	template<typename VecType>
	void addClause(const int prefix, const VecType & c) {
		if (isActive()) {
			write(prefix);
			for (int i = 0; i < c.size(); ++i)
				writeLit(c[i]);
			write(0);
		}
	}
	inline void writeLit(const Lit in) {
		unsigned l = toInt(in) + 2;
		assert(l > 0u && toInt(in) + 2 > 0);
		while (l > 127u) {
			write(128u + (l & 127u));
			l >>= 7u;
		}
		write(l);
	}

private:
	FILE * proof;

	inline void write(const int & c) {
		assert(proof != NULL);
		assert(c != EOF);
		if (putc_unlocked(c, proof) == EOF)
			assert(false);
	}
};
}

#endif /* MTL_DRATPRINT_H_ */

