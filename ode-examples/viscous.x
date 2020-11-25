# This example simulates a bead sliding on a smooth circular wire.  The
# output displays the height of the bead as a function of time.  There is
# viscous damping, so the bead should settle toward the equilibrium point
# (i.e. ordinate = 0).

# You may run this example by doing:
#
#    <viscous.x ode | graph-X -C
#
# or alternatively, to get a real-time plot,
#
#    <viscous.x ode | graph-X -C -x 0 20 -y 0 2.5

a = 1  # radius of circular wire
g = 10 # acceleration due to gravity
w = 10 # angular velocity of circular wire
b = 1  # damping coefficient

the' = vthe
vthe' = (w^2)*sin(the)*cos(the) - (g/a)*sin(the) - b * vthe
the = 0.1
vthe = 0

print t, the 

step 0, 20
