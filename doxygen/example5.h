/**
 * @page example5 Example 5 : Configurational forces
 * \dontinclude configurationalForces/bending2D/userFunctions.cc
 *
 * \htmlonly <style>div.image img[src="configForces2.png"]{width:15cm;}</style> \endhtmlonly
 * @image html configForces2.png
 *
 * In this code, we model the evolving configuration of a material as it undergoes bending.
 * We solve for two displacement fields. The configurational displacement field represents
 * the displacement due to evolving material configuration, and is modeled using
 * a nonconvex strain energy density function.
 *
 * \f{eqnarray*}{
 * \boldsymbol{\Theta} &=& \half(\boldsymbol{\chi}^T\boldsymbol{\chi} - \mathbbm{1})\\
 * \eta_1 &=& \Theta_{11} + \Theta_{22},\,
 * \eta_2 = \Theta_{11} - \Theta_{22},\,
 * \eta_6 = \Theta_{12}\\
 * \psi^\mathrm{M} &=& \frac{d}{s^2}\left(\eta_1^2 + \eta_6^2\right)
 * -\frac{2d}{s^2}\eta_2^2 + \frac{d}{s^4}\eta_2^4
 * + \frac{l^2d}{s^2}|\nabla^0 \eta_2|^2\\
 * \boldsymbol{D} &=& \frac{\partial \psi^\mathrm{M}}{\partial \boldsymbol{\chi}},\,
 * \boldsymbol{B} = \frac{\partial \psi^\mathrm{M}}{\partial \nabla^0\boldsymbol{\chi}}\\
 * \f}
 *
 * The standard displacement is governed by an anisotropic St. Venant-Kirchhoff model, where
 * the anisotropy is dependant on the configurational strain.
 *
 * \f{eqnarray*}{
 * \boldsymbol{E} &=& \frac{1}{2}(\boldsymbol{F}^T\boldsymbol{F} - \mathbbm{1})\\
 * \alpha_I(\boldsymbol{\chi}) &=& \alpha\Lambda_I(\boldsymbol{\chi}), \, \Lambda_I^2 = \sum_{i=1}^3\chi_{iI}^2\\
 * \mathbb{C}(\boldsymbol{\chi}) &=& \beta(\mathbbm{1}\otimes\mathbbm{1}) + 2\mu\mathbb{I} +
 * \sum_{I=1}^3 \left(\alpha_I(\boldsymbol{\chi}) -\beta - 2\mu \right)
 * \boldsymbol{e}_I\otimes \boldsymbol{e}_I\otimes\boldsymbol{e}_I\otimes \boldsymbol{e}_I\\
 * \psi^\mathrm{S} &=& \frac{1}{2} \boldsymbol{E}:\mathbb{C}(\boldsymbol{\chi}):\boldsymbol{E} \\
 * &=& \frac{1}{2}\left[\beta\mathrm{tr}(\boldsymbol{E})^2 + 2\mu(\boldsymbol{E}:\boldsymbol{E}) + 
 * \sum_{I=1}^3 E_{II}^2\left(\alpha_I(\boldsymbol{\chi}) -\beta - 2\mu \right) \right]\\
 * \frac{\partial \psi^\mathrm{S}}{\partial \boldsymbol{\chi}} &=& 
 * \frac{1}{2} \sum_{I=1}^3 \frac{\alpha}{\Lambda_I}\boldsymbol{\chi}(\boldsymbol{e}_I\otimes \boldsymbol{e}_I) E_{II}^2
 * \f}
 *
 * The total displacement is the sum of the configurational and standard displacements, and
 * is used to define boundary conditions. We solve the following weak form:
 *
 * \f{eqnarray*}{
 * 0 &=& \int \limits_{{\Omega}_0} \nabla^0\nabla^0\boldsymbol{W}\,\vdots\,\boldsymbol{B} \,\mathrm{d}V_0+\\
 * &\phantom{=}& +\int \limits_{{\Omega}_0} \nabla^0\boldsymbol{W}:\left[\boldsymbol{D}+J_\chi\left(\mathcal{E} \boldsymbol{\chi}^{-T}
 *  + \frac{\partial \psi^\mathrm{S}}{\partial \boldsymbol{\chi}} \right) \right] \, \mathrm{d}V_0\\
 * &\phantom{=}& + \int \limits_{{\Omega}_0} \nabla^0\bar{\boldsybmol{w}}:\left(J_\chi\boldsymbol{P} \boldsymbol{\chi}^{-T} \right)
 *  \, \mathrm{d}V_0
 * \f}
 *
 * With the current settings, the following structure is obtained (deformation has been scaled 10x):
 *
 * \htmlonly <style>div.image img[src="example5.png"]{width:15cm;}</style> \endhtmlonly
 * @image html example5.png 
 *
 * See the paper "A variational treatment of material configurations with application to interface motion and microstructural evolution",
 * G. Teichert, et al. (Journal of the Mechanics and Physics of Solids, 2017).
 *
 * Implementation: Level 1 users
 * ==============================
 *
 * To model evolving material configurations with two displacment fields, we will specify the following through defining user functions: <br>
 * - Boundary conditions <br>
 * - Load stepping <br>
 * - Derived fields for output <br>
 * - Constitutive model <br>
 * - Parameter values <br>
 * - Weak form of the PDE <br>
 *
 *
 * First, we include the header file declaring the required user functions. These functions will be defined in this file.
 *
 * \line userFunctions
 *
 * Now, we first define any optional user functions. Optional user functions have a default definition that can be 
 * redefined by the user using a function pointer.
 * This will be done in the \c defineParameters function. The available list of optional user functions includes:
 * \c boundaryConditions, \c scalarInitialConditions, \c vectorInitialConditions, \c loadStep, \c adaptiveTimeStep, and \c projectFields.
 * In this example, we redefine the \c boundaryConditions and \c projectFields functions, while using the default functions for the others.
 *
 * <b> The \c boundaryConditions function </b>
 *
 * This function defines Dirichlet boundary conditions using PetIGA's \c IGASetBoundaryValue function.
 * The arguments to this function are as follows: the iga object (user.iga),
 * the "axis" (0, 1, or 2, corresponding to the x, y, or z-axis),
 * the "side" (0 or 1), the "dof", and the "value" that is to be imposed.
 * Note that this can only set a uniform value for a degree-of-freedom on any side.
 * Here, we fix all displacements for both configurational and total displacement fields at x=0 (axis=0,side=0).
 * We also define vertical configurational and total displacements at x=10 (axis=0,side=1).
 * Note that the magnitude of the displacement is dependent on the \c scale parameter, which is defined in the next function, \c loadStep.
 *
 * \skip template
 * \until //end
 *
 * <b> The \c loadStep function </b>
 *
 * This function allows us to update the \c scale parameter and call the \c boundaryConditions function with the updated value
 * at every time step. This is useful when the Dirichlet boundary condition is too large to allow convergence when applied all at once.
 * Here, the value for /scale is taken from the current "time".
 *
 * \skip template
 * \until //end
 *
 * <b> The \c projectFields function </b>
 *
 * If there are field values derived from the solution fields that are of interest, we can compute these
 * values at each quadrature point and project the value to the nodes. Here, we compute the value \c eta2,
 * which is a function of the configurational displacement that reflects the local rectangular variant of the crystal structure.
 *
 * \skip template
 * \until //end
 *
 * <b> The \c constitutiveModel function </b>
 *
 * This function defines the standard and configurational stresses, strain energy densities, and kinematic values that appear in the resiudal.
 * Note that it is used only in this file (by the \c residual functions),
 * so it is not a class member function nor does it have an associated function pointer.
 *
 * \skip template
 * \until solutionVectors
 *
 * The first part of this function computes variables associated with the standard strain energy density function \f$\psi^\mathrm{S}\f$:
 *
 * \f{eqnarray*}{
 * \boldsymbol{E} &=& \frac{1}{2}(\boldsymbol{F}^T\boldsymbol{F} - \mathbbm{1})\\
 * \alpha_I(\boldsymbol{\chi}) &=& \alpha\Lambda_I(\boldsymbol{\chi}), \, \Lambda_I^2 = \sum_{i=1}^3\chi_{iI}^2\\
 * \mathbb{C}(\boldsymbol{\chi}) &=& \beta(\mathbbm{1}\otimes\mathbbm{1}) + 2\mu\mathbb{I} +
 * \sum_{I=1}^3 \left(\alpha_I(\boldsymbol{\chi}) -\beta - 2\mu \right)
 * \boldsymbol{e}_I\otimes \boldsymbol{e}_I\otimes\boldsymbol{e}_I\otimes \boldsymbol{e}_I\\
 * \psi^\mathrm{S} &=& \frac{1}{2} \boldsymbol{E}:\mathbb{C}(\boldsymbol{\chi}):\boldsymbol{E} \\
 * &=& \frac{1}{2}\left[\beta\mathrm{tr}(\boldsymbol{E})^2 + 2\mu(\boldsymbol{E}:\boldsymbol{E}) + 
 * \sum_{I=1}^3 E_{II}^2\left(\alpha_I(\boldsymbol{\chi}) -\beta - 2\mu \right) \right]\\
 * \frac{\partial \psi^\mathrm{S}}{\partial \boldsymbol{\chi}} &=& 
 * \frac{1}{2} \sum_{I=1}^3 \frac{\alpha}{\Lambda_I}\boldsymbol{\chi}(\boldsymbol{e}_I\otimes \boldsymbol{e}_I) E_{II}^2
 * \f}
 *
 * \skip mu
 * \until trans(F)*P
 *
 * The second part computes variables associated with the material strain energy density function \f$\psi^\mathrm{M}\f$:
 *
 * \f{eqnarray*}{
 * \boldsymbol{\Theta} &=& \half(\boldsymbol{\chi}^T\boldsymbol{\chi} - \mathbbm{1})\\
 * \eta_1 &=& \Theta_{11} + \Theta_{22},\,
 * \eta_2 = \Theta_{11} - \Theta_{22},\,
 * \eta_6 = \Theta_{12}\\
 * \psi^\mathrm{M} &=& \frac{d}{s^2}\left(\eta_1^2 + \eta_6^2\right)
 * -\frac{2d}{s^2}\eta_2^2 + \frac{d}{s^4}\eta_2^4
 * + \frac{l^2d}{s^2}|\nabla^0 \eta_2|^2\\
 * \boldsymbol{D} &=& \frac{\partial \psi^\mathrm{M}}{\partial \boldsymbol{\chi}},\,
 * \boldsymbol{B} = \frac{\partial \psi^\mathrm{M}}{\partial \nabla^0\boldsymbol{\chi}}\\
 * \f}
 *
 * \skip Es
 * \until //end
 *
 * <b> The \c defineParameters function </b>
 *
 * The user is required to define the \c defineParameters and \c residual functions. The \c defineParameters defines variables
 * and functions in the \c AppCtx object. The \c AppCtx object is defined
 * in the appCtx.h file. This function is used to define any values in \c user that will be needed in the problem.
 * It is also used to set any function pointers for user functions that we have redefined.
 *
 * \skip template
 * \until void
 *
 * We define the mesh, domain, and total applied displacement.
 *
 * \skip user.N[0]
 * \until user.uDirichlet
 *
 * We specify the number of vector and scalar solution and projection fields by adding the name of each field to
 * their respective vector. Here, we have two vector solution fields (the configurational displacement and total displacement) and one scalar projection field
 * (eta2). We do not use any scalar solution or vector projection fields in this example.
 *
 * \skip Displacement
 * \until "eta2"
 * 
 * We can specify the polynomial order of the basis splines, as well as the global continuity.
 * Note that the global continuity must be less than the polynomial order.
 * Here, we use a quadratic basis function with C-1 continuity because of the gradient elasticity terms.
 *
 * \skip polyOrder
 * \until globalContinuity
 *
 * We specify the value \c dtVal, which is a load step in this problem, as well as the \c totalTime, or total loading.
 * We can also specify a location (iteration number and time) to restart.
 *
 * \skip dtVal
 * \until RESTART_TIME
 *
 * Finally, we redirect the desired user function pointers to the \c boundaryConditions, \c projectFields, and \c loadStep functions that we
 * defined above. This completes the \c defineParameters function.
 *
 * \skip boundaryConditions
 * \until //end
 *
 * <b> The \c residual function </b>
 *
 * The residual function defines the residual that is to be driven to zero.
 * This is the central function of the code.
 * It is set up to follow the analytical weak form of the PDE.
 * It has a number of arguments that give problem information at the current quadrature point
 * (see Example 1 and the documentation for the arguments' classes for further information).
 *
 * The example code here implements the following weak form (neglecting body force and traction boundary conditions):
 *
 * \f{eqnarray*}{
 * 0 &=& \int \limits_{{\Omega}_0} \nabla^0\nabla^0\boldsymbol{W}\,\vdots\,\boldsymbol{B} \,\mathrm{d}V_0+\\
 * &\phantom{=}& +\int \limits_{{\Omega}_0} \nabla^0\boldsymbol{W}:\left[\boldsymbol{D}+J_\chi\left(\mathcal{E} \boldsymbol{\chi}^{-T}
 *  + \frac{\partial \psi^\mathrm{S}}{\partial \boldsymbol{\chi}} \right) \right] \, \mathrm{d}V_0\\
 * &\phantom{=}& + \int \limits_{{\Omega}_0} \nabla^0\bar{\boldsybmol{w}}:\left(J_\chi\boldsymbol{P} \boldsymbol{\chi}^{-T} \right)
 *  \, \mathrm{d}V_0
 * \f}
 *
 * After calling the \c constitutiveModel function to compute the necessary values,
 * we compute the residual in a manner very similar to the analytical weak form:
 *
 * \skip template
 * \until //end
 *
 * Finally, we include a file that instatiates the template functions \c defineParameters and \c residual. This bit of code
 * will generally be the same for any problem (unless you decide to use a different automatic differentation library),
 * the user does not need to modify it.
 *
 * \line "userFunctionsInstantiation.h"
 *
 * The complete implementation can be found at  <a href="https://github.com/mechanoChem/mechanoChemIGA/blob/master/initBounValProbs/configurationalForces/bending2D/userFunctions.cc">Github</a>.
 *
 * Parameters file: Interface for level 2 users
 * ==============================
 *
 * Now let's look at the parameters file, \c parameters.prm. The advantages of the parameters file are that
 * these values can be changed without recompiling the code and it can provide a clean interface to the code.
 * \dontinclude configurationalForces/bending2D/parameters.prm
 *
 * The parameters defined in the parameters file overwrite any previous values defined in the \c defineParameters function.
 * Anything following the pound sign (#) is a comment. A parameter is defined using the syntax: 
 *
 * \c set \c parameterName \c = \c parameterValue
 *
 * There is a set list of variables that can be read from the parameters file. Anything else will be added to 
 * the \c matParam structure with a double number type. Tensor objects can follow the format: 1 x 1 or [1,1] or (1,1), 
 * where the number of components must equal the spatial dimension of the problem.
 *
 * In this example file, we begin by specifying the spatial dimension, the geometry dimensions, and the mesh size:
 *
 * \skip dim
 * \until set N
 *
 * Next, we define some parameters related to loading:
 *
 * \skip Load
 * \until dtVal
 *
 * We then define restart information, output frequency, and spline parameters.
 *
 * \skip Restart
 * \until globalContinuity
 *
 * Note that we don't need to include all (or even any) of these parameters in this file. We defined default values previously.
 *
 * The complete parameters file can be found at  <a href="https://github.com/mechanoChem/mechanoChemIGA/blob/master/initBounValProbs/configurationalForces/bending2D/parameters.prm">Github</a>.
 *
 */
