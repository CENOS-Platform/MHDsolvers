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
    Foam::solvers::conductingFluid

Description
    Adds electromagnetic interaction to Foam::solvers::fluid

SourceFiles
    conductingFluid.C

\*---------------------------------------------------------------------------*/

#ifndef conductingFluid_H
#define conductingFluid_H

#include "isothermalFluid.H"
#include "electroBase.H"
#include "fluidThermoThermophysicalTransportModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class conductingFluid Declaration
\*---------------------------------------------------------------------------*/

class conductingFluid
:
    public isothermalFluid,
    public electroBase
{

protected:

    // Thermophysical transport

        autoPtr<fluidThermoThermophysicalTransportModel>
            thermophysicalTransport;

    //- Store old velocity field
        volVectorField U_old_;

    // Control parameters

        scalar maxDi = 1;

    // Time-step controls

        scalar DiNum = 0;


    // Protected Member Functions

        //- Read controls
        virtual void readControls();

        //- Local dummy turbulent alpha field
        volScalarField alphat_;

        //- Turbulent alpha field
        volScalarField& alphat;


private:

    // Private Member Functions

        //- Correct the cached Courant numbers
        void correctDiNum();

public:
        

    //- Runtime type information
    TypeName("conductingFluid");


    // Constructors

        //- Construct from region mesh
        conductingFluid(fvMesh& mesh);

        //- Disallow default bitwise copy construction
        conductingFluid(const conductingFluid&) = delete;


    //- Destructor
    virtual ~conductingFluid();


    // Member Functions
    
        //non-const Temperature access for initialization purposes
        volScalarField& getTemperature();

        //- Store old velocity field
        const volVectorField& U_old;
        //- Set U_old = U
        virtual void storeU();

        //- Return the current maximum time-step for stable solution
        virtual scalar maxDeltaT() const;

        //- Called at the start of the time-step, before the PIMPLE loop
        virtual void preSolve();

        //- Called at the start of the PIMPLE loop
        virtual void prePredictor();

        //- Construct and solve the energy equation,
        //  convert to temperature
        //  and update thermophysical and transport properties
        virtual void momentumPredictor();
        virtual void thermophysicalPredictor();

        //- Correct the momentum and thermophysical transport modelling
        virtual void postCorrector();

        //- Solve electromagnetic modelling
        virtual void solveElectromagnetics() override;

    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const conductingFluid&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
