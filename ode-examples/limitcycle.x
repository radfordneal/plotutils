# This example shows a nonlinear system with a stable limit cycle.
# You may run it by typing

# ode -f limcyc.x | graph-X -C -x -2 2 -y -2 2
# step 0,2
# step 2,4
# step 4,6
# step 6,8
# .

# An orbit converging on the limit cycle will be traced out incrementally.
# If you are using a color X Windows display, each segment of the orbit
# will be a different color.  This is a feature provided by graph-X, which
# normally changes the linemode after each dataset it reads. If you do not
# like this feature, you may turn it off by using `graph-X -B' instead of
# `graph-X'.

yone' = yone + ytwo - yone*(yone*yone+ytwo*ytwo)
ytwo' = -yone + ytwo - ytwo*(yone*yone+ytwo*ytwo)

yone = 1
ytwo = 2
print yone, ytwo
