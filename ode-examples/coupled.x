# This example simulates two similar pendulums, coupled with a horizontal
# spring.  The motion is taken to be undamped.
# 
# The plot shows the displacement of one of the pendulums, as a function of
# time.  It illustrates how energy (e.g. maximum displacement) oscillates
# back and forth between the two.

# You may run this example by doing:
#
#    <coupled.x ode | graph-X -C
# 
# or alternatively, to get a real-time plot,
#
#    <coupled.x ode | graph-X -C -x 0 50 -y -0.6 0.6

# The equations are:
# m*x1'' = - m*g*x1/l + k(x2 - x1)
# m*x2'' = - m*g*x2/l - k(x2 - x1)

xone' = vxone
xtwo' = vxtwo

vxone' = -g*xone/lone + k/mone * ( xtwo - xone)
vxtwo' = -g*xtwo/ltwo - k/ltwo * ( xtwo - xone)

k = 1
g = 9.8		# acceleration due to gravity
lone = 5
ltwo = 5
mone = 1
mtwo = 1

xone = 0.0
xtwo = 0.3
vxone = 0
vxtwo = 0

print t, xone
step 0,50
