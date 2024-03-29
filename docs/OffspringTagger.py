#!/usr/bin/env python

#
# $File: OffspringTagger.py $
#
# This file is part of simuPOP, a forward-time population genetics
# simulation environment. Please visit https://github.com/BoPeng/simuPOP
# for details.
#
# Copyright (C) 2004 - 2010 Bo Peng (Bo.Peng@bcm.edu)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

# This script is an example in the simuPOP user's guide. Please refer to
# the user's guide (https://github.com/BoPeng/simuPOP/manual) for a detailed
# description of this example.
#

import simuPOP as sim
pop = sim.Population(1000, loci=1, infoFields='offspring_idx')
pop.evolve(
    initOps=[
        sim.InitSex(),
        sim.InitGenotype(freq=[0.5, 0.5]),
    ],
    matingScheme=sim.RandomMating(ops=[
        sim.MendelianGenoTransmitter(),
        # lethal recessive alleles
        sim.MaSelector(loci=0, wildtype=0, fitness=[1, 0.90, 0.5]),
        sim.OffspringTagger(),
        sim.DiscardIf('offspring_idx > 4'),
    ], numOffspring=10),
    postOps=[
        sim.Stat(alleleFreq=0, step=10),
        sim.PyEval(r"'gen %d: allele freq: %.3f\n' % "
            "(gen, alleleFreq[0][1])", step=10)
    ],
    gen = 50,
)
