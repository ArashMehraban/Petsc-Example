How to compile:
 1) Set PETSC_DIR and PETSC_ARACH variables in your path
 2) make all

How to run (examples):
 For Q1 mesh on 2 processors:
   mpiexec -n 2 ./main -polydegree 1 -dof 3 -f cube8.exo

 For Q2 mesh on 2 processors:
   mpiexec -n 2 ./main -polydegree 2 -dof 3 -f cube8.exo
