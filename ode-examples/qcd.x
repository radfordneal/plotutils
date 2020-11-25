# You may run this example by doing
#
#    <qcd.x ode | graph-X -C
# 
# This simulates a coupled system of ordinary differential equations that
# arises in the mathematical theory of quantum chromodynamics (QCD).
#
# Source: Griffiths et al., J. Math. Phys.

f' = fp
fp' = -f*g*g
g' = gp
gp' = g*f*f

f = 0
fp = -1
g = 1
gp = -1

print t, f
step 0, 5
