static char help[] = "petsc-example-SEM ordering";

#include<petscdmplex.h>
#include <petscvec.h>


typedef struct{
  char          filename[PETSC_MAX_PATH_LEN]; //optional exodusII file
  PetscInt      polydegree;                   //polynomial degree {Q1, Q2,...} (1 less than number of interpolating points in 1D)
  PetscInt      dof;                          //dof per node
}AppCtx;


#undef __FUNCT__
#define __FUNCT__ "processUserOptions"
PetscErrorCode processUserOptions(MPI_Comm comm, AppCtx *userOptions)
{

  PetscErrorCode	ierr;
  PetscBool       fileflg;

  PetscFunctionBeginUser;
    ierr = PetscOptionsBegin(comm, "", "options", "DMPLEX");CHKERRQ(ierr);
      ierr = PetscOptionsString("-f", "Exodus.II filename to read", "main.c", userOptions->filename, userOptions->filename, sizeof(userOptions->filename), &fileflg);CHKERRQ(ierr);
      #if !defined(PETSC_HAVE_EXODUSII)
        if(flg)  SETERRQ(comm, PETSC_ERR_ARG_WRONG, "This option requires ExodusII support. Reconfigure your Arch with --download-exodusii");
      #endif
      //ERROR if no mesh file is provided
      if(fileflg == PETSC_FALSE)
        SETERRQ(PETSC_COMM_SELF, PETSC_ERR_SUP, "This code requires a mesh file (exodusII)");

      ierr = PetscOptionsInt("-polydegree", "Finite Element polynomial degree", "main", userOptions->polydegree, &userOptions->polydegree, NULL);CHKERRQ(ierr);
      ierr = PetscOptionsInt("-dof", "degrees of freedom", "main", userOptions->dof, &userOptions->dof, NULL);CHKERRQ(ierr);
    PetscOptionsEnd();
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "createDistributedDM"
PetscErrorCode createDistributedDM(MPI_Comm comm, AppCtx user, DM *dm){

  PetscErrorCode  ierr;
  const char      *filename = user.filename;
  PetscBool       interpolate = PETSC_FALSE;
  DM              distributedMesh = NULL;


  PetscFunctionBeginUser;
    if(user.polydegree == 2){
      interpolate = PETSC_TRUE;
    }

    ierr = DMPlexCreateFromFile(comm, filename, interpolate, dm);CHKERRQ(ierr);
    ierr = DMPlexDistribute(*dm, 0, NULL, &distributedMesh);CHKERRQ(ierr);
  	if (distributedMesh) {
  		ierr = DMDestroy(dm);CHKERRQ(ierr);
  		*dm  = distributedMesh;
  	}
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "dmCreateSection"
PetscErrorCode dmCreateSection(MPI_Comm comm, DM *dm, PetscInt numFields, PetscInt dof){

  PetscErrorCode ierr;
  PetscSection   section;
  PetscInt       pStart, pEnd;

  PetscFunctionBeginUser;
    ierr = PetscSectionCreate(comm,&section);CHKERRQ(ierr);
  	ierr = PetscSectionSetNumFields(section, numFields);CHKERRQ(ierr);
  	ierr = DMPlexGetChart(*dm, &pStart, &pEnd);CHKERRQ(ierr);
  	ierr = PetscSectionSetChart(section,pStart, pEnd);CHKERRQ(ierr);
    for(PetscInt i=pStart; i < pEnd; i++){
        ierr = PetscSectionSetDof(section, i, dof);CHKERRQ(ierr);
    }
  	ierr = PetscSectionSetUp(section);CHKERRQ(ierr);
    ierr = DMSetDefaultSection(*dm,section);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc, char **argv)
{
  PetscErrorCode    ierr;
  DM                dm;
  AppCtx	          user;
  PetscInt          numFields = 1; //3D elasticity for example
  Vec               Ul;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help); CHKERRQ(ierr);
    ierr = processUserOptions(PETSC_COMM_WORLD, &user);CHKERRQ(ierr);
    ierr = createDistributedDM(PETSC_COMM_WORLD, user, &dm);CHKERRQ(ierr);
    ierr = dmCreateSection(PETSC_COMM_WORLD, &dm, numFields, user.dof);CHKERRQ(ierr);
    ierr = DMGetLocalVector(dm,&Ul);CHKERRQ(ierr);
    //ierr = VecView(Ul,PETSC_VIEWER_STDOUT_SELF);CHKERRQ(ierr);
  ierr = PetscFinalize();CHKERRQ(ierr);
return 0;
}//end of main
