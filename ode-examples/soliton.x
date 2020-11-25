# You may run this example by doing:
#
#    <solit1.x ode | graph-X -C
#
# or alternatively, to create a real-time plot,
#
#    <solit1.x ode | graph-X -C -x 5 15 5 -y -3 3

# This example numerically solves an initial value problem arising in
# John Selker's soliton thesis:
# 
# x'' = (2 / t) * x' + B * x
# B = -1
# x(5) = 0
# x'(5) = 1

x' = vx
vx' = 2 / t * vx + B * x
B = -1

x = 0
vx = 1

print t, x
step 5, 15
