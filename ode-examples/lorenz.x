# This example displays a beautiful strange attractor: the Lorenz
# attractor.

# You may run this example by doing:
#
#    <lorenz.x ode | graph-X -C
#
# or alternatively, to get a real-time plot,
#
#    <lorenz.x ode | graph-X -C -x -10 10 -y -10 10
#
# You may also produce and print a Postscript version by doing
#
#    <lorenz.x ode | graph-ps -C -x -10 10 -y -10 10 -W 0 | lpr
#
# The `-W 0' sets the line width for the Postscript plot to
# be zero.  That means that the thinnest line possible will be used.

# The Lorenz model, a third order system.
# Interesting cases are r = 26, 2.5<t<30, x = z = 0, y = 1
# and r = 17, 1<t<50, x = z = 0, y = 1.
x' = -3*(x-y)
y' = -x*z+r*x-y
z' = x*y-z

r = 26
x = 0
y = 1
z = 0

print x, y
step 0, 200
