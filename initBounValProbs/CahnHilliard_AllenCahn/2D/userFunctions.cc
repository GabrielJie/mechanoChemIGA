#include "userFunctions.h"

template<unsigned int dim>
double userScalarInitialConditions(const Tensor<1,dim,double> &x, unsigned int scalar_i, AppCtx<dim> &user)
{

  switch(scalar_i) {
  case 0: return 0.5 + 0.01*(0.5 - (double)(rand() % 100 )/100.0); //Random about 0.5
  case 1: return 0.5 + 0.03*(0.5 - (double)(rand() % 100 )/100.0); //Random about 0.5
  default: return 0.;
  }

} //end scalarInitialConditions

template<typename T>
T F_eta(T c, T eta){
  //return std::pow(c-0.1,2)*std::pow(c-0.9,2) + 2.*(eta-0.2)*(eta-0.8)*(2.*eta-1.);
  return 10.*(eta - 0.8)*(std::pow(c-0.9,2) + 5.*std::pow(eta-0.2,2)) + 
    10.*(eta - 0.2)*(std::pow(c-0.1,2) + 5.*std::pow(eta-0.8,2));
}

template<typename T>
T F_cc(T c, T eta){
  //return 2*eta*(std::pow(2.*c-1.,2) + 2.*(c-0.1)*(c-.9));
  return 2.*c*(std::pow(c-0.1,2) + std::pow(c-0.9,2) + 5.*std::pow(eta-0.2,2) + 5.*std::pow(eta-0.8,2)) + 8.*(c-0.1)*(c-0.9);
}

template<typename T>
T F_ceta(T c, T eta){
  //return 2.*(c-0.1)*(c-0.9)*(2.*c-1.);
  return 20.*(std::pow(c-0.1,2)*std::pow(eta-0.2,2) + std::pow(c-0.9,2)*std::pow(eta-0.8,2));
} //end free energy derivative functions

template<unsigned int dim>
void defineParameters(AppCtx<dim>& user){
 
  user.N[0] = 20;
  user.N[1] = 20;
  user.L[0] = 1.;
  user.L[1] = 1.;

  //Set the domain to be periodic in the x and y directions
  user.periodic[0] = PETSC_TRUE;
  user.periodic[1] = PETSC_TRUE;

  //Define some material parameters (can be overwritten by parameters file)
  user.matParam["mobility"] = .1; //Mobility
  user.matParam["kinetic_coeff"] = 2.; //Kinetic coefficient
  user.matParam["kappa"] = .0005; //Gradient energy parameter

  user.matParam["influx"] = 0.; //Influx

  user.dtVal = .1;
  user.totalTime = 20;
  user.RESTART_IT = 0;
  user.RESTART_TIME = 0.;
  user.skipOutput = 2;

  user.scalarSolnFields.push_back("c");
  user.scalarSolnFields.push_back("eta");

  user.polyOrder = 2;
  user.globalContinuity = 1;

  user.scalarInitialConditions = userScalarInitialConditions;

} //end defineParameters

template<unsigned int dim, typename T>
void residual(bool dV,
	      bool dS,
	      const Tensor<1,dim,double> &x,
	      const Tensor<1,dim,double> &normal,
	      const solutionScalars<dim,T> &c,
	      const solutionVectors<dim,T> &u,
	      const testFunctionScalars<dim,T> &w1,
	      const testFunctionVectors<dim,T> &w2,
	      AppCtx<dim> &user,
	      Sacado::Fad::SimpleFad<T> &r){

  //Chemistry
  double dt = user.dt;
  double jn = user.matParam["influx"];
  double M = user.matParam["mobility"],
    L = user.matParam["kinetic_coeff"]; //Mobility, kinetic coefficient
  double kappa1 = user.matParam["kappa"],
    kappa2 = user.matParam["kappa"];
  double tau = 0.1*user.N[0]/user.L[0];
  
  //Get chemical potential and derivatives
  T f_cc, f_ceta, f_eta;
  f_cc = F_cc(c.val(0),c.val(1));
  f_ceta = F_ceta(c.val(0),c.val(1));
  f_eta = F_eta(c.val(0),c.val(1));
  
  //Cahn-Hilliard with Nitche's method
  r = ( w1.val(0)*(c.val(0) - c.valP(0))/dt )*dV;  
  r += ( M*w1.grad(0)*(f_cc*c.grad(0) + f_ceta*c.grad(1)))*dV;
  r += M*kappa1*w1.laplacian(0)*c.laplacian(0)*dV;
  
  r += -w1.val(0)*jn*dS; //Boundary flux
  r += -M*kappa1*( c.laplacian(0)*(w1.grad(0)*normal) + w1.laplacian(0)*(c.grad(0)*normal) )*dS;
  r += tau*(w1.grad(0)*normal)*(c.grad(0)*normal)*dS;
  
  //Allen-Cahn
  r += ( w1.val(1)*(c.val(1) - c.valP(1))/dt )*dV;
  r += L*( w1.val(1)*f_eta + kappa2*(w1.grad(1)*c.grad(1)) )*dV;
  
} //end residual

#include "userFunctionsInstantiation.h"
