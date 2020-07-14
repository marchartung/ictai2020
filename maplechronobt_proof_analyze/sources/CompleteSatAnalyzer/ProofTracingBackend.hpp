/*
 * Analyzer.h
 *
 *  Created on: 19.12.2019
 *      Author: hartung
 */

#ifndef COMPLETESATANALYZER_ANALYZER_H_
#define COMPLETESATANALYZER_ANALYZER_H_

#include "ClauseUsageTypes.h"
#include "ClauseDatabase.h"
#include "TmpVarState.h"
#include "Statistics.h"
#include "DratPrint.h"

#include <string>
#include <sstream>
#include <fstream>
#include <type_traits>

namespace CompleteSatAnalyzer {

template<bool enable>
class Analyzer {
	Analyzer() = delete;
	Analyzer(Analyzer const &) = delete;
	Analyzer(Analyzer &&) = delete;
	Analyzer& operator=(Analyzer const &) = delete;
	Analyzer& operator=(Analyzer &&) = delete;
public:

	constexpr Analyzer(bool const analyze,
			std::string const & clauseDatabaseName, uint64_t const maxBytes) :
			printHuman(false), analyze(analyze), isCdclSolving(false), solution(
			l_Undef), numVars(0), nOpenResolved(0), numInitCl(0), emptyCid(
					Cid_Undef), preProcessEnd(Cid_Undef), nConflicts(0), cd(
					(analyze) ? clauseDatabaseName : std::string("")) {
		if (isActive()) {
			std::cout << "c Sat Analyzer will analyze result" << std::endl;
		} else
			std::cout << "c Analyzer is deactivated by mode: " << !enable
					<< " and dyn: " << !analyze << std::endl;
	}

	~Analyzer() {
		if (isActive())
			analyzeAndPrint();
	}

	constexpr bool isActive() const {
		return enable && analyze;
	}

	void enableLRAT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		lratFilename = filename;
		std::cout << "c Analyzer will dump LRAT to '" << filename << "'"
				<< std::endl;
	}

	void enableDRAT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		dratFilename = filename;
		std::cout << "c Analyzer will dump DRAT to '" << filename << "'"
				<< std::endl;
	}

	void enableGRIT(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		gritFilename = filename;
		std::cout << "c Analyzer will dump GRIT to '" << filename << "'"
				<< std::endl;
	}

	void enableTrace(std::string const & filename) {
		if (!isActive() || filename.empty())
			return;
		traceFilename = filename;
		std::cout << "c Analyzer will dump TRACE to '" << filename << "'"
				<< std::endl;
	}

	void analyzeAndPrint() {
		if (!isActive())
			return;
		assert(cd.good());
		double const anaStartTime = cpuTime();
		if (solution != l_Undef) {
			if (solution == l_False) {
				analyzeUnsat();
				std::cout << "Marking proof: " << cpuTime() - anaStartTime
						<< " seconds" << std::endl;
			}
		} else
			std::cout << "c no solution. Stats are incomplete" << std::endl;
		if (!traceFilename.empty())
			collectTrace();
		if (!lratFilename.empty())
			collectLrat();
		if (!dratFilename.empty())
			collectDrat();
		if (!gritFilename.empty())
			collectGrit();
		std::cout << "c Analyze time: 	" << cpuTime() - anaStartTime
				<< " seconds" << std::endl;
		std::cout << "c Analyze used disk storage:	"
				<< static_cast<double>(cd.getUsedBytes()) / 1000.0 / 1000.0
				<< " MB" << std::endl;
	}

	void addResolvent(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedResolvents.push(cid);
		}
	}

	void addUnorderedResolvent(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedResolvents.push(cid);
			++nOpenResolved;
		}
	}

	void addUnitResolvent(Cid const & cid) {
		if (isActive()) {
			assert(cid != Cid_Undef);
			collectedUnits.push(cid);
		}
	}

	template<typename ClauseType>
	Cid addClause(ClauseType & c, unsigned const lbd = 0, ClauseOrigin origin =
			ClauseOrigin::UNDEF) {
		Cid res = Cid_Undef;
		if (isActive()) {
			assert(emptyCid == Cid_Undef);
			if (origin == ClauseOrigin::UNDEF && !hasResolvents())
				origin = ClauseOrigin::INITIAL;
			assert(origin != ClauseOrigin::INITIAL || !isCdclSolving);
			mergeResolvents();
			res = cd.addClause(nConflicts, c, collectedResolvents, lbd,
					nOpenResolved, origin);
			if (origin == ClauseOrigin::INITIAL)
				cd[res].setProofId(++numInitCl);
			if (nOpenResolved == 0)
				assert(isGritOrdered(cd[res]));
			clearResolvents();
			assert(checkResolution(res));
			if (c.size() == 0)
				emptyCid = res;
//			std::cout << "added:";
//			printClause(c);
		}
		return res;
	}

	void mergeResolvents() {
		for (int i = 0; i < collectedUnits.size(); ++i)
			collectedResolvents.push(collectedUnits[i]);
	}

	Clause const & getClause(Cid const & cid) const {
		return cd[cid];
	}

	Cid addEmptyClause(ClauseOrigin const origin = ClauseOrigin::UNDEF) {
		if (isActive()) {
			assert(hasResolvents());
			vec<Lit> c;
			mergeResolvents();
			emptyCid = cd.addClause(nConflicts, c, collectedResolvents, 0,nOpenResolved,
					origin);
			assert(checkResolution(emptyCid));
			clearResolvents();
		}
		return emptyCid;
	}

	Cid addUnit(Lit const l, ClauseOrigin origin = ClauseOrigin::UNDEF) {
		Cid res = Cid_Undef;
		if (isActive()) {
			assert(emptyCid == Cid_Undef);
			if (origin == ClauseOrigin::UNDEF && !hasResolvents()) {
				origin = ClauseOrigin::INITIAL;
			}
			vec<Lit> c(1, l);
			assert(origin != ClauseOrigin::INITIAL || !isCdclSolving);
			mergeResolvents();
			res = cd.addClause(nConflicts, c, collectedResolvents, 0, nOpenResolved, origin);
			if (origin == ClauseOrigin::INITIAL)
				cd[res].setProofId(++numInitCl);
			clearResolvents();
			assert(checkResolution(res));
		}
		return res;
	}

	void removeClause(Cid const cid) {
		if (isActive()) {
			cd.addDeleteClause(nConflicts, cid);
		}
	}

	void anounceRevertableResolvents() {
		if (isActive()) {
			wasOpenResolved.push(nOpenResolved);
			revertUnitResolvents.push(collectedUnits.size());
			revertResolvents.push(collectedResolvents.size());
		}
	}

	void clearRevertableResolvents() {
		if (isActive()) {
			assert(revertResolvents.size() > 0);
			int const shrinkBy = collectedResolvents.size()
					- revertResolvents.last();
			int const shrinkByUnits = collectedUnits.size()
					- revertUnitResolvents.last();
			nOpenResolved = wasOpenResolved.last();
			wasOpenResolved.pop();
			revertResolvents.pop();
			revertUnitResolvents.pop();
			collectedResolvents.shrink_(shrinkBy);
			collectedUnits.shrink_(shrinkByUnits);
		}
	}

	void clearResolvents() {
		if (isActive()) {
			nOpenResolved = 0;
			wasOpenResolved.clear();
			collectedResolvents.clear();
			collectedUnits.clear();
			revertResolvents.clear();
			revertUnitResolvents.clear();
		}
	}

	bool hasResolvents() const {
		return collectedResolvents.size() > 0 || collectedUnits.size() > 0;
	}

	void notifyConflictEnd() {
		if (isActive()) {
			assert(!hasResolvents());
			++nConflicts;
		}
	}

	void notifyPreprocessorEnd() {
		if (!isCdclSolving) {
			isCdclSolving = true;
			preProcessEnd = nConflicts;
		}
	}

	void newVar() {
		++numVars;
	}

	void notifySolution(lbool const sol) {
		if (isActive()) {
			solution = sol;
			assert(
					sol != l_False || ( emptyCid != Cid_Undef && cd[emptyCid].nLits() == 0));
		}
	}

	template<typename ClauseType>
	bool equal(Cid const cid, ClauseType const & c) const {
		Clause const & c2 = cd[cid];
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

private:

	struct OnlineGroupStat {
		uint64_t nClauses;
		uint64_t nConflicts;
		uint64_t nFlips;
		OnlineGroupStat() :
				nClauses(0), nConflicts(0), nFlips(0) {
		}
		void add(OnlineGroupStat const & g) {
			nClauses += g.nClauses;
			nConflicts += g.nConflicts;
			nFlips += g.nFlips;
		}

		std::string toString() const {
			std::stringstream ss;
			ss << "nCl:" << nClauses << " conflicts: " << nConflicts
					<< " flips:" << nFlips << " flips/cl:"
					<< average(nFlips, nClauses);
			return ss.str();
		}

		std::string toCsv(std::string const & sep = ",") const {
			std::stringstream ss;
			ss << nClauses << sep << nConflicts << sep << nFlips;
			return ss.str();
		}

		static std::string csvHeader(std::string const & sep = ",") {
			std::stringstream ss;
			return std::string("nCl") + sep + std::string("nConfl") + sep
					+ std::string("nFlip");
		}
	};

	bool const printHuman;
	bool const analyze;
	bool isCdclSolving;
	lbool solution;
	int numVars;

	unsigned nOpenResolved;
	unsigned numInitCl;
	Cid emptyCid;
	Cid preProcessEnd;

	uint64_t nConflicts;
	std::string lratFilename;
	std::string gritFilename;
	std::string traceFilename;
	std::string dratFilename;

	vec<Cid> collectedResolvents;
	vec<Cid> collectedUnits;
	vec<Cid> tmpOrdered;
	vec<unsigned> tmpResolvents;
	vec<unsigned> wasOpenResolved;
	vec<int> revertResolvents;
	vec<int> revertUnitResolvents;

	//temporaries:
	struct Seen {
		bool seen;
		bool unit;
		unsigned nApp;
		Seen() {
			reset();
		}

		void reset() {
			seen = false;
			unit = false;
			nApp = 0;
		}
	};
	mutable vec<int> toClear;
	mutable vec<Seen> seenVars;
	TmpVarStates tmpState;

	struct AppCount {
		bool set;
		bool inClause;
		bool withSign;
		unsigned pos;
		unsigned neg;
		AppCount() :
				set(false), inClause(false), withSign(false), pos(0), neg(0) {
		}
		void clear() {
			set = false;
			inClause = false;
			withSign = false;
			pos = 0;
			neg = 0;
		}
	};
	mutable vec<AppCount> apps;

	ClauseDatabase cd;

	void makeGritOrder(Clause & c) {
		if (tmpState.size() != numVars)
			tmpState.resize(numVars);

		tmpOrdered.clear();
		tmpState.setMaxDeps(c.nResolvents());
		for (Clause::size_type i = 0; i < c.nLits(); ++i)
			tmpState.setSeen(var(c.getLit(i)));

		for (Clause::size_type i = 0; i < c.nResolvents(); ++i)
			tmpResolvents.push(i);

		unsigned nAdded = 0;
		while (tmpResolvents.size() > 0) {
			bool isAsserting = true;
			int idx = -1;
			unsigned const rid = tmpResolvents.last();
			tmpResolvents.pop();
			Clause & r = cd[c.getResolvent(rid)];
			for (Clause::size_type j = 0; j < r.nLits(); ++j)
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
				tmpOrdered.push(c.getResolvent(rid));
				if (idx != -1) {
					Var const v = var(r[idx]);
					assert(!tmpState.isSeen(v));
					tmpState.setSeen(v);
					if (tmpState.hasDeps(v)) {
						auto const & deps = tmpState.varDeps(var(r[idx]));
						for (int j = 0; j < deps.size(); ++j) {
							if (tmpState.removeDep(deps[j]) < 2
									&& deps[j] != rid)
								tmpResolvents.push(deps[j]);
						}
					}
				}
			}
		}
		tmpState.clear();
		assert(nAdded == c.nResolvents());
		c.setOpen(0);
		for (unsigned i = 0; i < nAdded; ++i)
			c.getResolvent(i) = tmpOrdered[nAdded - 1 - i];
	}

	bool isGritOrdered(Clause const & c) {
		if (tmpState.size() != numVars)
			tmpState.resize(numVars);

		for (Clause::size_type i = 0; i < c.nLits(); ++i)
			tmpState.setSeen(var(c.getLit(i)));

		for (Clause::size_type i = c.nResolvents() - 1; i < c.nResolvents();
				--i) {
			Clause const & r = cd[c.getResolvent(i)];
			unsigned numUndef = 0;
			for (int j = 0; j < r.size(); ++j)
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

	void dumpGritClause(std::ofstream & ous, Clause & c) {
		assert(c.isUsedInProof());
		assert(!c.isDeleteClause());
		assert(c.nLits() > 0 || (emptyCid != Cid_Undef && &cd[emptyCid] == &c));

		assert(isGritOrdered(c));

		ous << c.getProofId() << " ";
		for (int i = 0; i < c.size(); ++i)
			ous << ((sign(c[i])) ? "-" : "") << var(c[i]) + 1 << " ";
		ous << "0 ";

		for (Clause::size_type i = c.nResolvents() - 1; i < c.nResolvents();
				--i) {
			Cid const cid = c.getResolvent(i);
			assert(
					cd[cid].getProofId()
							!= std::numeric_limits<uint32_t>::max());
			assert(cd[cid].isUsedInProof());
			ous << cd[cid].getProofId() << " ";
		}
		ous << "0\n";
	}

	unsigned numResolvedLits(Clause const & c) const {
		unsigned score = 0;
		for (Clause::size_type i = 0; i < c.nResolvents(); ++i) {
			Clause const & r = cd[c.getResolvent(i)];
			if (r.nLits() == 1) {
				int const v = var(r.getLit(0));
				if (v >= seenVars.size())
					seenVars.growTo(v + 1);
				seenVars[v].unit = true;
				assert(
						score
								>= ((seenVars[v].nApp > 2) ?
										seenVars[v].nApp - 2 : 0));
				score -= (seenVars[v].nApp > 2) ? seenVars[v].nApp - 2 : 0;
			}
			for (Clause::size_type j = 0; j < r.nLits(); ++j) {
				int const v = var(r.getLit(j));

				if (v >= seenVars.size())
					seenVars.growTo(v + 1);

				++seenVars[v].nApp;
				if (seenVars[v].nApp > 2 && !seenVars[v].unit)
					++score;
				if (!seenVars[v].seen) {
					toClear.push(v);
					seenVars[v].seen = true;
				}
			}
		}

		for (int i = 0; i < toClear.size(); ++i)
			seenVars[toClear[i]].reset();
		toClear.clear();

		return score;
	}

	template<bool reorder>
	void backtrackSolution(vec<Cid> & cids) {
		while (cids.size() > 0) {
			Cid const cid = cids.last();
			cids.pop();
			Clause & c = cd[cid];
			assert(!c.isDeleteClause());
			assert(c.isUsedInProof());
			for (Clause::size_type i = 0; i < c.nResolvents(); ++i) {
				Cid const rid = c.getResolvent(i);
				Clause & r = cd[rid];
				if (!r.isUsedInProof()) {

					r.setUsedInProof(true);
					cids.push(rid);
				}
			}
			if (reorder && c.isOpenResolved())
				makeGritOrder(c);

		}
	}

	unsigned countNonTrivialResolvents(Clause const & c) const {
		unsigned res = 0;
		for (Clause::size_type i = 0; i < c.nResolvents(); ++i)
			if (cd[c.getResolvent(i)].nLits() > 2)
				++res;
		return res;
	}
	unsigned maxLevelResolventLevel(Clause const & c) const {
		unsigned res = 0;
		for (Clause::size_type i = 0; i < c.nResolvents(); ++i)
			if (cd[c.getResolvent(i)].getLevel() > res)
				res = cd[c.getResolvent(i)].getLevel();
		return res;
	}

	void collectDrat() {
		double const anaStartTime = cpuTime();
		DratPrint drat(true, dratFilename);
		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause & c = cd[cid];

			bool const addClause = !c.isDeleteClause() && c.isUsedInProof()
					&& c.getGroup() != ClauseOrigin::REDUNDANT
					&& c.getGroup() != ClauseOrigin::INITIAL;
			if (addClause) {
				drat.addClause(c);
			} else {
				bool const delClInProof = (c.isUsedInProof()
						&& c.isDeleteClause()
						&& c.getGroup() != ClauseOrigin::INITIAL)
						|| (c.getGroup() == ClauseOrigin::INITIAL
								&& !c.isUsedInProof() && !c.isDeleteClause());
				if (delClInProof)
					drat.removeClause(
							(c.isDeleteClause()) ? cd[c.getResolvent(0)] : c);
			}
		}
		std::cout << "c Dumping DRAT: " << cpuTime() - anaStartTime
				<< " seconds" << std::endl;
	}

	void checkedOpen(std::ofstream & ous, std::string const & name) {
		ous.open(name);
		if (!ous.good()) {
			std::cout << "c Error: Could not open LRAT file '" << name
					<< "' for output." << std::endl;
			throw "";
		}
	}

	void collectGrit() {
		double const anaStartTime = cpuTime();
		uint32_t curId = numInitCl;
		std::ofstream grit;
		checkedOpen(grit, gritFilename);

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause & c = cd[cid];

			if (c.isUsedInProof() && !c.isDeleteClause()) {
				if (c.getGroup() != ClauseOrigin::INITIAL)
					c.setProofId(++curId);
				dumpGritClause(grit, c);
			}
		}
		grit.close();
		std::cout << "c Dumping GRIT: " << cpuTime() - anaStartTime
				<< " seconds" << std::endl;
	}

	void collectTrace() {
		double const anaStartTime = cpuTime();
		uint32_t curId = numInitCl;
		std::ofstream trace;
		checkedOpen(trace, traceFilename);

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			Clause & c = cd[cid];

			if (c.isUsedInProof() && !c.isDeleteClause()) {
				if (c.getGroup() != ClauseOrigin::INITIAL)
					c.setProofId(curId++);
				dumpGritClause(trace, c);
			}
		}
		trace.close();
		std::cout << "c Dumping TRACE: " << cpuTime() - anaStartTime
				<< " seconds" << std::endl;
	}

	void collectLrat() {
		double const anaStartTime = cpuTime();
		uint32_t curId = numInitCl;
		std::ofstream lratOus;
		checkedOpen(lratOus, lratFilename);

		vec<uint32_t> deleteCls;

		for (Cid cid = cd.begin(); cid != cd.end(); cd.next(cid)) {
			bool const isDeleteClause = cd[cid].isDeleteClause();
			Clause & c = cd[(isDeleteClause) ? cd[cid].getResolvent(0) : cid];
			bool const addClause = !isDeleteClause && c.isUsedInProof()
					&& c.getGroup() != ClauseOrigin::INITIAL;
			if (addClause) {
				if (deleteCls.size() > 0) {
					lratOus << curId << " d ";
					for (int i = 0; i < deleteCls.size(); ++i)
						lratOus << deleteCls[i] << " ";
					lratOus << "0\n";
					deleteCls.clear();
				}
				c.setProofId(++curId);
				dumpGritClause(lratOus, c);
			} else {
				bool const delClInProof = (c.isUsedInProof() && isDeleteClause
						&& c.getGroup() != ClauseOrigin::INITIAL)
						|| (c.getGroup() == ClauseOrigin::INITIAL
								&& !c.isUsedInProof() && !isDeleteClause);
				if (delClInProof)
					deleteCls.push(
							((c.isDeleteClause()) ? cd[c.getResolvent(0)] : c).getProofId());
			}
		}
		lratOus.close();
		std::cout << "c Dumping LRAT: " << cpuTime() - anaStartTime
				<< " seconds" << std::endl;
	}
	void analyzeUnsat() {
		assert(!hasResolvents());
		vec<Cid> & cids = collectedResolvents;
		if (emptyCid == Cid_Undef)
			std::cout
					<< "c Warning: Solution found but no empty clause detected"
					<< std::endl;
		assert(emptyCid != Cid_Undef);
		assert(cd[emptyCid].nLits() == 0);
		cids.push(emptyCid);
		cd[emptyCid].setUsedInProof(true);

		if (!gritFilename.empty() || !lratFilename.empty())
			backtrackSolution<true>(cids);
		else
			backtrackSolution<false>(cids);
	}

	void print(Clause const & c) const {
		std::cout << "[";
		for (Clause::size_type i = 0; i < c.nLits(); ++i)
			std::cout << ((sign(c.getLit(i))) ? "-" : "") << var(c.getLit(i))
					<< " ";
		std::cout << "]\n";
	}

	void printClause(Clause const & c, bool const useStates = false) const {
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

	void printClause(Minisat::Clause const & c) const {
		std::cout << "[";
		for (int i = 0; i < c.size(); ++i)
			std::cout << ((sign(c[i])) ? "-" : "") << var(c[i]) << " ";
		std::cout << "]\n";
	}

	void traversClause(Clause const & c) const {
		for (Lit const * l = c.lbegin(); l != c.lend(); ++l) {
			Var const v = var(*l);
			if (v >= apps.size()) {
				apps.growTo(v + 1);
			}
			AppCount & a = apps[v];
			a.inClause = true;
			a.withSign = sign(*l);
			a.set = true;
//				std::cout << ((sign(l)) ? "-" : "") << var(l) << " in clause\n";
		}
	}

	void traversResolvents(Clause const & c) const {
		for (Cid const * r = c.rbegin(); r != c.rend(); ++r) {
			Clause const & rc = cd[*r];
			for (Lit const * l = rc.lbegin(); l != rc.lend(); ++l) {
				Var const v = var(*l);
				if (v >= apps.size()) {
					apps.growTo(v + 1);
				}
				AppCount & a = apps[v];
				bool const s = sign(*l);
				a.neg += s;
				a.pos += !s;
				a.set = true;
			}
		}
	}

	bool checkClause(Clause const & c) const {
		for (Lit const * l = c.lbegin(); l != c.lend(); ++l) {
			AppCount & a = apps[var(*l)];
			bool correct = true;
			if (a.neg > 0 && a.pos > 0) {
				std::cout << "Literal in clause but actually resolved\n";
				assert(false);
				return false;
			}
			if ((a.neg > 0 && !a.withSign) || (a.pos > 0 && a.withSign)) {
				std::cout << "sign not correct\n";
				assert(false);
				return false;
			}
			if (!correct) {
				std::cout << "wrong lit: " << var(*l) << "\n";
				std::cout << "wrong learned:";
				printClause(c);
				assert(false);
				return false;
			}
			a.clear();
		}
		return true;
	}

	bool checkResolvents(Clause const & c) const {
		for (Cid const * r = c.rbegin(); r != c.rend(); ++r) {
			Clause const & rc = cd[*r];
			for (Lit const * l = rc.lbegin(); l != rc.lend(); ++l) {

				AppCount & a = apps[var(*l)];
				if (!a.set)
					continue;
				assert(!a.inClause);

				if ((a.neg == 0 || a.pos == 0)) {
					std::cout << "literal " << var(*l)
							<< " not resolved, but also not in clause\n";
					std::cout << "learned:";
					printClause(c);
					std::cout << "wrong:";
					printClause(rc);
					assert(false);
					return false;
				}
				a.clear();
			}
		}
		return true;
	}
	bool checkResolution(Cid const cid) const {
		Clause const & c = cd[cid];
		traversClause(c);
		traversResolvents(c);
		bool correct = checkClause(c);
		if (correct)
			correct = checkResolvents(c);
		return correct;
	}
}
;

}

#endif /* COMPLETESATANALYZER_ANALYZER_H_ */
