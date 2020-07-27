/*
 * DratPrint.h
 *
 *  Created on: 10.04.2019
 *      Author: hartung
 */

#ifndef MTL_DRATPRINT_H_
#define MTL_DRATPRINT_H_

#include "ClauseUsageTypes.hpp"
#include <cstdio>
#include <string>
#include <iostream>

namespace PTB {

template<typename Lit, typename ToDimacs>
class DratPrint {
public:
	DratPrint(const bool shouldPrint, const std::string & proofFileName, ToDimacs toDimacs) :
			proof((shouldPrint) ? fopen(proofFileName.c_str(), "wb") : NULL), toDimacs(toDimacs) {
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
			for (unsigned i = 0; i < c.size(); ++i)
				if (c[i] != l)
					writeLit(c[i]);
			write(0);
		}
	}

	void addEmptyClause() {
		addClause('a', std::vector<Lit>());
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
			for (unsigned i = 0; i < c.size(); ++i)
				writeLit(c[i]);
			write(0);
		}
	}
	inline void writeLit(const Lit in) {
		unsigned l = 2*toDimacs(in);
		l = (l<0) ? -l + 1 : l;
		assert(l > 0u && toInt(in) + 2 > 0);
		while (l > 127u) {
			write(128u + (l & 127u));
			l >>= 7u;
		}
		write(l);
	}

private:
	FILE * proof;
	ToDimacs toDimacs;

	inline void write(const int & c) {
		assert(proof != NULL);
		assert(c != EOF);
		if (putc_unlocked(c, proof) == EOF)
			assert(false);
	}
};
}

#endif /* MTL_DRATPRINT_H_ */

