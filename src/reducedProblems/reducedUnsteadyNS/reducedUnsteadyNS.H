/*---------------------------------------------------------------------------*\
     ██╗████████╗██╗  ██╗ █████╗  ██████╗ █████╗       ███████╗██╗   ██╗
     ██║╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗      ██╔════╝██║   ██║
     ██║   ██║   ███████║███████║██║     ███████║█████╗█████╗  ██║   ██║
     ██║   ██║   ██╔══██║██╔══██║██║     ██╔══██║╚════╝██╔══╝  ╚██╗ ██╔╝
     ██║   ██║   ██║  ██║██║  ██║╚██████╗██║  ██║      ██║      ╚████╔╝
     ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝      ╚═╝       ╚═══╝

 * In real Time Highly Advanced Computational Applications for Finite Volumes
 * Copyright (C) 2017 by the ITHACA-FV authors
-------------------------------------------------------------------------------
License
    This file is part of ITHACA-FV
    ITHACA-FV is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    ITHACA-FV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with ITHACA-FV. If not, see <http://www.gnu.org/licenses/>.
Class
    reducedProblem
Description
    A reduced problem for the unsteady NS equations
SourceFiles
    reducedUnsteadyNS.C
\*---------------------------------------------------------------------------*/

/// \file
/// Header file of the reducedUnsteadyNS class.
/// \dir
/// Directory containing the header and source files for the reducedUnsteadyNS class

#ifndef reducedUnsteadyNS_H
#define reducedUnsteadyNS_H

#include "fvCFD.H"
#include "IOmanip.H"
#include "reducedSteadyNS.H"
#include "unsteadyNS.H"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

/// Newton object for the resolution of the reduced problem using a supremizer approach
struct newton_unsteadyNS_sup: public newton_argument<double>
{
    public:
        newton_unsteadyNS_sup() {}
        newton_unsteadyNS_sup(int Nx, int Ny,
                              unsteadyNS& problem): newton_argument<double>(Nx, Ny),
            problem(&problem),
            Nphi_u(problem.NUmodes + problem.liftfield.size() + problem.NSUPmodes),
            Nphi_p(problem.NPmodes),
            N_BC(problem.inletIndex.rows())
        {}

        int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;
        int df(const Eigen::VectorXd& x,  Eigen::MatrixXd& fjac) const;

        int Nphi_u;
        int Nphi_p;
        int N_BC;

        scalar nu;
        scalar dt;
        Eigen::VectorXd y_old;
        Eigen::VectorXd BC;

        unsteadyNS* problem;
};


/// Newton object for the resolution of the reduced problem using a PPE approach
struct newton_unsteadyNS_PPE: public newton_argument<double>
{
    public:
        newton_unsteadyNS_PPE() {}
        newton_unsteadyNS_PPE(int Nx, int Ny,
                              unsteadyNS& problem): newton_argument<double>(Nx, Ny),
            problem(&problem),
            Nphi_u(problem.NUmodes + problem.liftfield.size()),
            Nphi_p(problem.NPmodes),
            N_BC(problem.inletIndex.rows())
        {}

        int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;
        int df(const Eigen::VectorXd& x,  Eigen::MatrixXd& fjac) const;

        int Nphi_u;
        int Nphi_p;
        int N_BC;

        scalar nu;
        scalar dt;
        Eigen::VectorXd y_old;
        Eigen::VectorXd BC;

        unsteadyNS* problem;
};


/*---------------------------------------------------------------------------*\
                        Class reducedProblem Declaration
\*---------------------------------------------------------------------------*/

/// Class where it is implemented a reduced problem for the <b> unsteady Navier-stokes </b> problem.
/** In this class are implemented the methods to solve a reduced version of the stationary Navier-stokes
equations. This class must be used together with the steadyNS class */
class reducedUnsteadyNS: public reducedSteadyNS
{
    private:

    public:
        // Constructors
        /// Construct Null
        reducedUnsteadyNS();
        /// Construct Null
        ///
        /// @param      problem  a full order unsteadyNS problem
        ///
        explicit reducedUnsteadyNS(unsteadyNS& problem);

        ~reducedUnsteadyNS() {};

        /// Function object to call the non linear solver sup approach
        newton_unsteadyNS_sup newton_object_sup;

        /// Function object to call the non linear solver PPE approach
        newton_unsteadyNS_PPE newton_object_PPE;

        /// Scalar to store the current time
        scalar time;

        /// Scalar to store the time increment
        scalar dt;

        /// Scalar to store the final time if the online simulation
        scalar finalTime;

        /// Scalar to store the initial time if the online simulation
        scalar tstart;

        /// Pointer to the FOM problem
        unsteadyNS* problem;

        // Functions


        /// Method to perform an online solve using a PPE stabilisation method
        ///
        /// @param[in]  vel_now   The vector of online velocity. It is defined in
        /// with an Eigen::MatrixXd and must have one col and as many rows as the number
        /// of parametrized boundary conditions.
        /// @param[in]  startSnap The first snapshot taken from the offline snahpshots
        /// and used to get the reduced initial condition.
        ///
        void solveOnline_PPE(Eigen::MatrixXd& vel_now, label startSnap = 0);

        /// Method to perform an online solve using a supremizer stabilisation method
        ///
        /// @param[in]  vel_now   The vector of online velocity. It is defined in
        /// with an Eigen::MatrixXd and must have one col and as many rows as the number
        /// of parametrized boundary conditions.
        /// @param[in]  startSnap The first snapshot taken from the offline snahpshots
        /// and used to get the reduced initial condition.
        ///
        void solveOnline_sup(Eigen::MatrixXd& vel_now, label startSnap = 0);

        /// Method to reconstruct a solution from an online solve with a PPE stabilisation technique.
        /// stabilisation method
        ///
        /// @param[in]  folder      The folder where to output the solutions
        /// @param[in]  printevery  The printevery
        ///
        void reconstruct_PPE(fileName folder = "./online_rec", int printevery = 1);

        /// Method to reconstruct the solutions from an online solve with a supremizer stabilisation technique.
        /// stabilisation method
        ///
        /// @param[in]  folder      The folder where to output the solutions
        /// @param[in]  printevery  The printevery
        ///
        void reconstruct_sup(fileName folder = "./online_rec", int printevery = 1);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



#endif

