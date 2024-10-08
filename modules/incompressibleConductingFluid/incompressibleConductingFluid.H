/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2022-2023 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::solvers::incompressibleFluid

Description
    Solver module for thermal transport in incompressibleFluid domains and regions for
    conjugate heat transfer, HVAC and similar applications, with optional mesh
    motion and mesh topology changes.

SourceFiles
    incompressibleConductingFluid.C

\*---------------------------------------------------------------------------*/

#ifndef incompressibleConductingFluid_H
#define incompressibleConductingFluid_H

#include "incompressibleFluid.H"
#include "electroBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class incompressibleConductingFluid Declaration
\*---------------------------------------------------------------------------*/

class incompressibleConductingFluid
:
    public incompressibleFluid,
    public electroBase
{

protected:

    //- Store old velocity field
        volVectorField U_old_;

    // density

        dimensionedScalar rho_;

public:

    // density

        const dimensionedScalar& rho;


    //- Runtime type information
    TypeName("incompressibleConductingFluid");


    // Constructors

        //- Construct from region mesh
        incompressibleConductingFluid(fvMesh& mesh);

        //- Disallow default bitwise copy construction
        incompressibleConductingFluid(const incompressibleConductingFluid&) = delete;


    //- Destructor
    virtual ~incompressibleConductingFluid();


    // Member Functions

    //- Store old velocity field
        const volVectorField& U_old;
        //- Set U_old = U
        virtual void storeU();

        // update transport properties
        virtual void momentumPredictor();

        //- Correct the momentum and electromagnetic modelling
        virtual void postCorrector();

        //- Solve electromagnetic modelling
        virtual void solveElectromagnetics();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const incompressibleConductingFluid&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
