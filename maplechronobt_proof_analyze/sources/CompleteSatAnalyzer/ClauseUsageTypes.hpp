/*
 * ClauseUsageTypes.h
 *
 *  Created on: 12.12.2019
 *      Author: hartung
 */

#ifndef CORE_CLAUSEUSAGETYPES_H_
#define CORE_CLAUSEUSAGETYPES_H_

#include <cstdint>
#include <cassert>
#include <string>
#include <vector>
#include <array>
#include <limits>

#include "MmapVector.h"
#include "core/SolverTypes.h"
#include "utils/System.h"

namespace CompleteSatAnalyzer {

using Lit=Minisat::Lit;

template<typename T>
using vec=Minisat::vec<T>;

using Minisat::var;
using Minisat::lit_Undef;

using Minisat::cpuTime;

using Var = Minisat::Var;
using Minisat::sign;

using lbool=Minisat::lbool;

}

#endif /* CORE_CLAUSEUSAGETYPES_H_ */
