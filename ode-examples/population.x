# This is an example from population biology (the predator-prey
# equations of Lotka and Volterra).  You may run it by doing:
#
#    <pop.x ode | graph-X -C
# 
# or alternatively, to get a real-time plot,
#
#    <pop.x ode | graph-X -C -x 0 10 -y 0 3
#
# The plot shows the population of the prey as a function of time.
# The curve oscillates, because as the prey die out, the predators
# starve.

# The differential equations are:
# x' = (A - By) x
# y' = (Cx - D) y
# A,B,C,D > 0

x' = (A - B*y) * x
y' = (C*x - D) * y

A = 1
B = 1
C = 1
D = 1

x = 3
y = 1

print t, x
step 0, 10
