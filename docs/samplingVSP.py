#!/usr/bin/env python

#
# $File: samplingVSP.py $
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
# create an age-structured population with a disease
import random
pop = sim.Population(10000, loci=10, infoFields='age')
sim.initGenotype(pop, freq=[0.3, 0.7])
sim.initInfo(pop, lambda: random.randint(0, 70), infoFields='age')
pop.setVirtualSplitter(sim.InfoSplitter(cutoff=(40, 60), field='age'))
sim.maPenetrance(pop, loci=5, penetrance=(0.1, 0.2, 0.3))
#
from simuPOP.sampling import drawCaseControlSample
sample = drawCaseControlSample(pop, cases=500, controls=500, subPops=[(0,1)])
ageInSample = sample.indInfo('age')
print(min(ageInSample), max(ageInSample))
