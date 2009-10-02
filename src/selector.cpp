/**
 *  $File: selector.cpp $
 *  $LastChangedDate$
 *  $Rev$
 *
 *  This file is part of simuPOP, a forward-time population genetics
 *  simulation environment. Please visit http://simupop.sourceforge.net
 *  for details.
 *
 *  Copyright (C) 2004 - 2009 Bo Peng (bpeng@mdanderson.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "selector.h"

namespace simuPOP {
bool selector::apply(population & pop)
{
	UINT fit_id = pop.infoIdx(this->infoField(0));

	subPopList subPops = applicableSubPops();

	if (subPops.empty()) {
		IndInfoIterator fitness = pop.infoBegin(fit_id);
		// fitness may change with generation so pass generation information
		for (IndIterator it = pop.indIterator(); it.valid(); ++it)
			*fitness++ = indFitness(& * it, pop.gen()) ;
		pop.turnOnSelection();
	} else {
		for (subPopList::iterator sp = subPops.begin(); sp != subPops.end(); ++sp) {
			IndInfoIterator fitness = pop.infoBegin(fit_id, *sp);
			DBG_FAILIF(static_cast<UINT>(sp->subPop()) > pop.numSubPop(), IndexError,
				"Wrong subpopulation index" + toStr(sp->subPop()) + " (number of subpops is " +
				toStr(pop.numSubPop()) + ")");
			for (IndIterator it = pop.indIterator(sp->subPop()); it.valid(); ++it)
				*fitness++ = indFitness(& * it, pop.gen());
			pop.turnOnSelection(sp->subPop());
		}
	}

	return true;
}


double mapSelector::indFitness(individual * ind, ULONG gen)
{
	size_t ply = ind->ploidy();

	vectori alleles(ply * m_loci.size());
	size_t idx = 0;

	for (vectoru::iterator loc = m_loci.begin(); loc != m_loci.end(); ++loc) {
		for (size_t p = 0; p < ply; ++p, ++idx)
			alleles[idx] = ind->allele(*loc, p);
		// if no phase, sort alleles...
		if (!m_phase && ply > 1) {
			if (ply == 2) {
				if (alleles[idx - 2] > alleles[idx - 1]) { // swap
					int tmp = alleles[idx - 2];
					alleles[idx - 2] = alleles[idx - 1];
					alleles[idx - 1] = tmp;
				}
			} else
				std::sort(alleles.begin() + idx - ply, alleles.end());
		}
	}

	tupleDict::iterator pos = m_dict.find(alleles);

	if (pos == m_dict.end()) {
		string allele_string = "(";
		for (size_t i = 0; i < alleles.size(); ++i) {
			if (i != 0)
				allele_string += ", ";
			allele_string += toStr(alleles[i]);
		}
		allele_string += ")";
		DBG_ASSERT(false, ValueError,
			"No fitness value for genotype " + allele_string);
	}

	return pos->second;
}


// currently assuming diploid
double maSelector::indFitness(individual * ind, ULONG gen)
{
	UINT index = 0;
	bool singleST = m_wildtype.size() == 1;

	for (vectoru::iterator loc = m_loci.begin(); loc != m_loci.end(); ++loc) {
		// get genotype of ind
		Allele a = ToAllele(ind->allele(*loc, 0));
		Allele b = ToAllele(ind->allele(*loc, 1));

		int numWildtype = 0;

		// count number of wildtype
		// this improve the performance a little bit
		if (singleST) {
			numWildtype = (AlleleUnsigned(a) == m_wildtype[0]) + (AlleleUnsigned(b) == m_wildtype[0]);
		} else {
			if (find(m_wildtype.begin(), m_wildtype.end(), AlleleUnsigned(a)) != m_wildtype.end() )
				numWildtype++;

			if (find(m_wildtype.begin(), m_wildtype.end(), AlleleUnsigned(b)) != m_wildtype.end() )
				numWildtype++;
		}

		index = index * 3 + 2 - numWildtype;
	}
	return m_fitness[index];
}


double mlSelector::indFitness(individual * ind, ULONG gen)
{
	if (m_mode == Multiplicative) {
		double fit = 1;
		for (opList::iterator s = m_selectors.begin(), sEnd = m_selectors.end();
		     s != sEnd; ++s)
			fit *= static_cast<selector * >(*s)->indFitness(ind, gen);
		return fit;
	} else if (m_mode == Additive) {
		double fit = 1;
		for (opList::iterator s = m_selectors.begin(), sEnd = m_selectors.end();
		     s != sEnd; ++s)
			fit -= 1 - static_cast<selector * >(*s)->indFitness(ind, gen);
		return fit < 0 ? 0. : fit;
	} else if (m_mode == Heterogeneity) {
		double fit = 1;
		for (opList::iterator s = m_selectors.begin(), sEnd = m_selectors.end();
		     s != sEnd; ++s)
			fit *= 1 - static_cast<selector * >(*s)->indFitness(ind, gen);
		return 1 - fit;
	}
	// this is the case for none.
	return 1.0;
}


double pySelector::indFitness(individual * ind, ULONG gen)
{
	if (m_len == 0) {
		m_len = m_loci.size() * ind->ploidy();
		m_alleles.resize(m_len);
		m_numArray = Allele_Vec_As_NumArray(m_alleles.begin(), m_alleles.end() );
	}

	DBG_FAILIF(static_cast<size_t>(m_len) != ind->ploidy() * m_loci.size(),
		SystemError,
		"Length of m_len is wrong. Have you changed pop type?");

	UINT pEnd = ind->ploidy();
	for (size_t i = 0, iEnd = m_loci.size(), j = 0; i < iEnd; ++i)
		for (UINT p = 0; p < pEnd; ++p)
			m_alleles[j++] = ToAllele(ind->allele(m_loci[i], p));

	if (infoSize() > 1) {
		if (m_info.size() + 1 != infoSize()) {
			m_info.resize(infoSize() - 1);
			m_infoArray = Double_Vec_As_NumArray(m_info.begin(), m_info.end());
		}
		// assign information fields from individusl
		for (size_t i = 1; i < infoSize(); ++i)
			m_info[i - 1] = ind->info(infoField(i));
	}


	if (infoSize() <= 1)
		return m_func(PyObj_As_Double, "(Oi)", m_numArray, gen);
	else
		return m_func(PyObj_As_Double, "(OiO)", m_numArray, gen, m_infoArray);
	return 0.;
}


}
