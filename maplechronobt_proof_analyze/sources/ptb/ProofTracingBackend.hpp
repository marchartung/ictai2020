/*
 * Analyzer.h
 *
 *  Created on: 19.12.2019
 *      Author: hartung
 */

#ifndef COMPLETESATANALYZER_ANALYZER_H_
#define COMPLETESATANALYZER_ANALYZER_H_

#include "ClauseDatabase.hpp"
#include "ClauseUsageTypes.hpp"
#include "DratPrint.hpp"
#include "Timer.hpp"
#include "TmpVarState.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <type_traits>

namespace PTB {

template<bool enable, typename Lit, typename LitToDimacs>
class Analyzer {
	Analyzer() = delete;
	Analyzer(Analyzer const &) = delete;
	Analyzer(Analyzer &&) = delete;
	Analyzer& operator=(Analyzer const &) = delete;
	Analyzer& operator=(Analyzer &&) = delete;
public:

	constexpr Analyzer(bool const analyze,
			std::string const & clauseDatabaseName, uint64_t const maxBytes,
			LitToDimacs toProof, int const verbosity = 1) :
			printHuman(false), analyze(analyze), numVars(0), verbosity(
					verbosity), nUnordered(0), nRat(0), numInitCl(0), toDimacs(
					toProof), cd(
					(analyze) ? clauseDatabaseName : std::string("")) {

		if (verbosity > 0) {
			if (isActive()) {
				std::cout << "c Sat Analyzer will analyze result" << std::endl;
			} else if (!analyze)
				std::cout << "c Analyzer is dynamically deactivated"
						<< std::endl;
			else if (!enable)
				std::cout << "c Analyzer was deactivated during compile time"
						<< std::endl;
		}
	}

	~Analyzer() {
	}

	constexpr bool isActive() const {
		return enable && analyze;
	}

	void enableLRAT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		lratFilename = filename;
		if (verbosity > 0)
			std::cout << "c Analyzer will dump LRAT to '" << filename << "'"
					<< std::endl;
	}

	void enableDRAT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		dratFilename = filename;
		if (verbosity > 0)
			std::cout << "c Analyzer will dump DRAT to '" << filename << "'"
					<< std::endl;
	}

	void enableGRIT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		gritFilename = filename;
		if (verbosity > 0)
			std::cout << "c Analyzer will dump GRIT to '" << filename << "'"
					<< std::endl;
	}

	void enableTrace(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		traceFilename = filename;
		if (verbosity > 0)
			std::cout << "c Analyzer will dump TRACE to '" << filename << "'"
					<< std::endl;
	}

	void newVar() {
		++numVars;
	}

	void addHint(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedResolvents.push_back(cid);
		}
	}

	void addUnorderedHint(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedResolvents.push_back(cid);
			++nUnordered;
		}
	}

	void addRatHint(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedResolvents.push_back(cd.markRat(cid));
			++nRat;
		}
	}

	void addUnitHint(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedUnits.push_back(cid);
		}
	}

	template<typename ClauseType>
	Cid addResolvent(ClauseType & c,
			ClauseOrigin origin = ClauseOrigin::UNDEF) {
		Cid res = Cid_Undef;
		if (isActive()) {
			origin = (origin == ClauseOrigin::UNDEF) ? getOrigin() : origin;
			mergeResolvents();
			res = cd.addClause(c, collectedResolvents, origin);
			if (origin == ClauseOrigin::INITIAL)
				cd[res].setProofId(++numInitCl);

			clearResolvents();
			if (c.size() == 0)
				analyzeAndPrint(res);
		}
		return res;
	}

	void removeResolvent(Cid const cid) {
		if (isActive()) {
			cd.addDeleteClause(cid);
		}
	}

	void markRevertableHints() {
		if (isActive())
			reverts.push_back(getRevertMarker());
	}

	void revertHints() {
		if (isActive()) {
			assert(reverts.size() > 0);

			RevertMarker const & m = reverts.back();
			nRat = m.nRat;
			nUnordered = m.nUnordered;
			collectedUnits.resize(m.nUnitHints);
			collectedResolvents.resize(m.nHints);
			reverts.pop_back();
		}
	}

private:

	typedef unsigned Var;

	bool const printHuman;
	bool const analyze;
	int numVars;
	int verbosity;

	unsigned nUnordered;
	unsigned nRat;
	unsigned numInitCl;

	LitToDimacs toDimacs;

	std::string lratFilename;
	std::string gritFilename;
	std::string traceFilename;
	std::string dratFilename;

	std::vector<Cid> collectedResolvents;
	std::vector<Cid> collectedUnits;
	std::vector<Cid> tmpOrdered;
	std::vector<unsigned> tmpResolvents;

	struct RevertMarker {
		unsigned nUnordered;
		unsigned nRat;
		unsigned nUnitHints;
		unsigned nHints;
		RevertMarker(unsigned nUnordered, unsigned nRat, unsigned nUnitHints,
				unsigned nHints) :
				nUnordered(nUnordered), nRat(nRat), nUnitHints(nUnitHints), nHints(
						nHints) {
		}
	};

	std::vector<RevertMarker> reverts;

	TmpVarStates tmpState;
	ClauseDatabase<Lit> cd;

	Var var(Lit const l) const {
		return std::abs(toDimacs(l)) - 1;
	}

	RevertMarker getRevertMarker() const {
		return RevertMarker(nUnordered, nRat, collectedUnits.size(),
				collectedResolvents.size());
	}

	void mergeResolvents() {
		for (unsigned i = 0; i < collectedUnits.size(); ++i)
			collectedResolvents.push_back(collectedUnits[i]);
	}

	void clearResolvents() {
		if (isActive()) {
			nUnordered = 0;
			nRat = 0;
			collectedResolvents.clear();
			collectedUnits.clear();
			reverts.clear();
		}
	}

	bool hasResolvents() const {
		return collectedResolvents.size() > 0 || collectedUnits.size() > 0;
	}

	ClauseOrigin getOrigin() const {
		ClauseOrigin res = ClauseOrigin::UNDEF;
		if (!hasResolvents())
			res = ClauseOrigin::INITIAL;
		else {
			assert(nUnordered == 0 || nRat == 0);
			if (nRat != 0)
				res = ClauseOrigin::RAT;
			else if (nUnordered != 0)
				res = ClauseOrigin::UNORDERED;
			else
				res = ClauseOrigin::ORDERED;
		}
		return res;
	}

	void analyzeAndPrint(Cid const emptyCid) {
		if (!isActive())
			return;
		assert(cd.good());
		Timer h;
		analyzeUnsat(emptyCid);
		if (verbosity > 0)
			std::cout << "Marking proof: " << h.getSeconds() << " seconds"
					<< std::endl;
		if (!traceFilename.empty())
			collectTrace();
		if (!lratFilename.empty())
			collectLrat();
		if (!dratFilename.empty())
			collectDrat();
		if (!gritFilename.empty())
			collectGrit();

		if (verbosity > 0) {
			std::cout << "c Analyze time: 	" << h.getSeconds() << " seconds"
					<< std::endl;
			std::cout << "c Analyze used disk storage:	"
					<< static_cast<double>(cd.getUsedBytes()) / 1000.0 / 1000.0
					<< " MB" << std::endl;
		}
	}

	template<typename ClauseType>
	bool equal(Cid const cid, ClauseType const & c) const {
		Clause<Lit> const & c2 = cd[cid];
		if ((unsigned) c.size() != c2.nLits()) {
			assert(false);
			return false;
		}
		for (int i = 0; i < c.size(); ++i) {
			bool found = false;
			for (unsigned j = 0; j < c2.nLits(); ++j)
				found |= c[i] == c2.getLit(j);
			if (!found) {
				printClause(c2);
				printClause(c);
				assert(false);
				return false;
			}
		}
		return true;
	}
	void makeRatOrdered(Clause<Lit> & c) {
		std::vector<Cid> cids(c.nHints());
		for (typename Clause<Lit>::size_type i = 0; i < c.nHints(); ++i)
			cids[i] = c.getHint(i);

		std::sort(cids.begin(), cids.end(), [](Cid const & a, Cid const & b) {
			return Cid_Internal::cast(a).id < Cid_Internal::cast(b).id;
		});
		for (typename Clause<Lit>::size_type i = 0; i < c.nHints(); ++i)
			c.getHint(i) = cids[i];
	}

	void makeUpOrdered(Clause<Lit> & c) {
		if (tmpState.size() != numVars)
			tmpState.resize(numVars);

		tmpOrdered.clear();
		tmpState.setMaxDeps(c.nHints());
		for (typename Clause<Lit>::size_type i = 0; i < c.nLits(); ++i)
			tmpState.setSeen(var(c.getLit(i)));

		for (typename Clause<Lit>::size_type i = 0; i < c.nHints(); ++i)
			tmpResolvents.push_back(i);

		unsigned nAdded = 0;
		while (tmpResolvents.size() > 0) {
			bool isAsserting = true;
			int idx = -1;
			unsigned const rid = tmpResolvents.back();
			tmpResolvents.pop_back();
			Clause<Lit> & r = cd[c.getHint(rid)];
			for (typename Clause<Lit>::size_type j = 0; j < r.nLits(); ++j)
				if (!tmpState.isSeen(var(r[j]))) {
					if (idx != -1) {
						if (isAsserting)
							tmpState.addDep(var(r[idx]), rid);
						tmpState.addDep(var(r[j]), rid);
						isAsserting = false;
					} else
						idx = j;
				}
			if (isAsserting) {
				++nAdded;
				assert(idx != -1 || tmpResolvents.size() == 0);
				tmpOrdered.push_back(c.getHint(rid));
				if (idx != -1) {
					Var const v = var(r[idx]);
					assert(!tmpState.isSeen(v));
					tmpState.setSeen(v);
					if (tmpState.hasDeps(v)) {
						auto const & deps = tmpState.varDeps(var(r[idx]));
						for (unsigned j = 0; j < deps.size(); ++j) {
							if (tmpState.removeDep(deps[j]) < 2
									&& deps[j] != rid)
								tmpResolvents.push_back(deps[j]);
						}
					}
				}
			}
		}
		tmpState.clear();
		assert(nAdded == c.nHints());
		c.setType(ClauseOrigin::ORDERED);
		for (unsigned i = 0; i < nAdded; ++i)
			c.getHint(i) = tmpOrdered[nAdded - 1 - i];
	}

	bool isGritOrdered(Clause<Lit> const & c) {
		if (tmpState.size() != numVars)
			tmpState.resize(numVars);

		for (typename Clause<Lit>::size_type i = 0; i < c.nLits(); ++i)
			tmpState.setSeen(var(c.getLit(i)));

		for (typename Clause<Lit>::size_type i = c.nHints() - 1; i < c.nHints();
				--i) {
			Clause<Lit> const & r = cd[c.getHint(i)];
			unsigned numUndef = 0;
			for (unsigned j = 0; j < r.size(); ++j)
				if (!tmpState.isSeen(var(r[j]))) {
					++numUndef;
					tmpState.setSeen(var(r[j]));
				}
			if (numUndef > 1) {
				tmpState.clear();
				return false;
			}
		}
		tmpState.clear();
		return true;
	}

	void dumpGritClause(std::ofstream & ous, Clause<Lit> & c) {
		assert(c.isUsedInProof());
		assert(!c.isDeleteClause());

		assert(isGritOrdered(c));
		assert(!c.isRat());

		ous << c.getProofId() << " ";
		for (typename Clause<Lit>::size_type i = 0; i < c.size(); ++i)
			ous << ((sign(c[i])) ? "-" : "") << var(c[i]) + 1 << " ";
		ous << "0 ";

		for (typename Clause<Lit>::size_type i = c.nHints() - 1; i < c.nHints();
				--i) {
			Cid const cid = c.getHint(i);
			assert(
					cd[cid].getProofId()
							!= std::numeric_limits<uint32_t>::max());
			assert(cd[cid].isUsedInProof());
			ous << cd[cid].getProofId() << " ";
		}
		ous << "0\n";
	}
	void dumpLratClause(std::ofstream & ous, Clause<Lit> & c) {
		assert(c.isUsedInProof());
		assert(!c.isDeleteClause());

		assert(isGritOrdered(c));
		bool const isRat = c.isRat();

		ous << c.getProofId() << " ";
		for (typename Clause<Lit>::size_type i = 0; i < c.size(); ++i)
			ous << ((sign(c[i])) ? "-" : "") << var(c[i]) + 1 << " ";
		ous << "0 ";

		for (typename Clause<Lit>::size_type i = c.nHints() - 1; i < c.nHints();
				--i) {
			Cid const cid = c.getHint(i);
			assert(!isRat || cd.isRat(cid));
			assert(
					cd[cid].getProofId()
							!= std::numeric_limits<uint32_t>::max());
			assert(cd[cid].isUsedInProof());
			ous << ((isRat) ? "-" : "") << cd[cid].getProofId() << " ";
		}
		ous << "0\n";
	}

	template<bool reorder>
	void backtrackSolution(std::vector<Cid> & cids) {
		while (cids.size() > 0) {
			Cid const cid = cids.back();
			cids.pop_back();
			Clause<Lit> & c = cd[cid];
			assert(!c.isDeleteClause());
			assert(c.isUsedInProof());
			for (typename Clause<Lit>::size_type i = 0; i < c.nHints(); ++i) {
				Cid const rid = c.getHint(i);
				Clause<Lit> & r = cd[rid];
				if (!r.isUsedInProof()) {

					r.setUsedInProof(true);
					cids.push_back(rid);
				}
			}
			if (reorder) {
				if (c.isUnordered())
					makeUpOrdered(c);
				else if (c.isRat())
					makeRatOrdered(c);
			}

		}
	}

	void collectDrat() {
		Timer h;
		DratPrint<Lit, LitToDimacs> drat(true, dratFilename, toDimacs);
		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause<Lit> & c = cd[cid];

			bool const addClause = !c.isDeleteClause() && c.isUsedInProof()
					&& c.getType() != ClauseOrigin::REDUNDANT
					&& c.getType() != ClauseOrigin::INITIAL;
			if (addClause) {
				drat.addClause(c);
			} else {
				bool const delClInProof = (c.isUsedInProof()
						&& c.isDeleteClause()
						&& c.getType() != ClauseOrigin::INITIAL)
						|| (c.getType() == ClauseOrigin::INITIAL
								&& !c.isUsedInProof() && !c.isDeleteClause());
				if (delClInProof)
					drat.removeClause(
							(c.isDeleteClause()) ? cd[c.getHint(0)] : c);
			}
		}
		if (verbosity > 0)
			std::cout << "c Dumping DRAT: " << h.getSeconds() << " seconds"
					<< std::endl;
	}

	void checkedOpen(std::ofstream & ous, std::string const & name) {
		ous.open(name);
		if (!ous.good()) {
			std::cerr << "c Error: Could not open LRAT file '" << name
					<< "' for output." << std::endl;
			throw "";
		}
	}

	void collectGrit() {
		Timer h;
		uint32_t curId = numInitCl;
		std::ofstream grit;
		checkedOpen(grit, gritFilename);

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause<Lit> & c = cd[cid];

			if (c.isUsedInProof() && !c.isDeleteClause()) {
				if (c.getType() != ClauseOrigin::INITIAL)
					c.setProofId(++curId);
				dumpGritClause(grit, c);
			}
		}
		grit.close();
		if (verbosity > 0)
			std::cout << "c Dumping GRIT: " << h.getSeconds() << " seconds"
					<< std::endl;
	}

	void collectTrace() {
		Timer h;
		uint32_t curId = numInitCl;
		std::ofstream trace;
		checkedOpen(trace, traceFilename);

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause<Lit> & c = cd[cid];

			if (c.isUsedInProof() && !c.isDeleteClause()) {
				if (c.getType() != ClauseOrigin::INITIAL)
					c.setProofId(curId++);
				dumpGritClause(trace, c);
			}
		}
		trace.close();
		if (verbosity > 0)
			std::cout << "c Dumping TRACE: " << h.getSeconds() << " seconds"
					<< std::endl;
	}

	void collectLrat() {
		Timer h;
		uint32_t curId = numInitCl;
		std::ofstream lratOus;
		checkedOpen(lratOus, lratFilename);

		std::vector<uint32_t> deleteCls;

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			bool const isDeleteClause = cd[cid].isDeleteClause();
			Clause<Lit> & c = cd[(isDeleteClause) ? cd[cid].getHint(0) : cid];
			bool const addClause = !isDeleteClause && c.isUsedInProof()
					&& c.getType() != ClauseOrigin::INITIAL;
			if (addClause) {
				if (deleteCls.size() > 0) {
					lratOus << curId << " d ";
					for (unsigned i = 0; i < deleteCls.size(); ++i)
						lratOus << deleteCls[i] << " ";
					lratOus << "0\n";
					deleteCls.clear();
				}
				c.setProofId(++curId);
				dumpLratClause(lratOus, c);
			} else {
				bool const delClInProof = (c.isUsedInProof() && isDeleteClause
						&& c.getType() != ClauseOrigin::INITIAL)
						|| (c.getType() == ClauseOrigin::INITIAL
								&& !c.isUsedInProof() && !isDeleteClause);
				if (delClInProof)
					deleteCls.push_back(
							((c.isDeleteClause()) ? cd[c.getHint(0)] : c).getProofId());
			}
		}
		lratOus.close();
		if (verbosity > 0)
			std::cout << "c Dumping LRAT: " << h.getSeconds() << " seconds"
					<< std::endl;
	}
	void analyzeUnsat(Cid const emptyCid) {
		assert(!hasResolvents());
		std::vector<Cid> & cids = collectedResolvents;

		assert(emptyCid != Cid_Undef);
		assert(cd[emptyCid].nLits() == 0);

		cids.push_back(emptyCid);
		cd[emptyCid].setUsedInProof(true);

		if (!gritFilename.empty() || !lratFilename.empty())
			backtrackSolution<true>(cids);
		else
			backtrackSolution<false>(cids);
	}

	void print(Clause<Lit> const & c) const {
		std::cout << "[";
		for (typename Clause<Lit>::size_type i = 0; i < c.nLits(); ++i)
			std::cout << ((sign(c.getLit(i))) ? "-" : "") << var(c.getLit(i))
					<< " ";
		std::cout << "]\n";
	}

	void printClause(Clause<Lit> const & c,
			bool const useStates = false) const {
		std::cout << "[";
		if (useStates) {
			for (unsigned i = 0; i < c.nLits(); ++i)
				std::cout << ((sign(c.getLit(i))) ? "-" : "")
						<< var(c.getLit(i)) << "("
						<< ((tmpState.isSeen(var(c.getLit(i)))) ? "s" : "u")
						<< ") ";
		} else {
			for (unsigned i = 0; i < c.nLits(); ++i)
				std::cout << ((sign(c.getLit(i))) ? "-" : "")
						<< var(c.getLit(i)) << " ";
		}
		std::cout << "]\n";
	}
}
;

}

#endif /* COMPLETESATANALYZER_ANALYZER_H_ */
