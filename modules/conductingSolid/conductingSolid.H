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
    Foam::solvers::conductingSolid

Description
    Adds electromagnetic interaction to Foam::solvers::solid

SourceFiles
    conductingSolid.C

\*---------------------------------------------------------------------------*/

#ifndef conductingSolid_H
#define conductingSolid_H

#include "solid.H"
#include "electroBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class conductingSolid Declaration
\*---------------------------------------------------------------------------*/

class conductingSolid
:
    public solid,
    public electroBase
{

//protected:

public:

    // Public Data


    //- Runtime type information
    TypeName("conductingSolid");


    // Constructors

        //- Construct from region mesh
        conductingSolid(fvMesh& mesh);

        //- Disallow default bitwise copy construction
        conductingSolid(const conductingSolid&) = delete;


    //- Destructor
    virtual ~conductingSolid();


    // Member Functions

        //- Construct and solve the energy equation,
        //  convert to temperature
        //  and update thermophysical and transport properties
        virtual void thermophysicalPredictor();
    
        //non-const Temperature access for initialization purposes
        volScalarField& getTemperature();

        //- Correct the momentum and thermophysical transport modelling
        virtual void postCorrector();

        //- Solve electromagnetic system
        virtual void solveElectromagnetics() override;

    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const conductingSolid&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
