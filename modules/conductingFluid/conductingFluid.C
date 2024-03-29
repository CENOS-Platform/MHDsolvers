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

\*---------------------------------------------------------------------------*/

#include "conductingFluid.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
    defineTypeNameAndDebug(conductingFluid, 0);
    addToRunTimeSelectionTable(solver, conductingFluid, fvMesh);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solvers::conductingFluid::conductingFluid(fvMesh& mesh)
:
    isothermalFluid(mesh),

    thermophysicalTransport
    (
        fluidThermoThermophysicalTransportModel::New
        (
            momentumTransport(),
            thermo
        )
    ),

    JxB_
    (
        IOobject
        (
            "JxB",
            runTime.name(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector(dimensionSet(1, -2, -2, 0, 0, 0, 0), Foam::vector(0,0,0))
    ),

    JJsigma_
    (
        IOobject
        (
            "JJsigma",
            runTime.name(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar(dimensionSet(1, -1, -3, 0, 0, 0, 0), 0)
    ),
    JxB(JxB_),
    JJsigma(JJsigma_)
{
    thermo.validate(type(), "h", "e");
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::conductingFluid::~conductingFluid()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


void Foam::solvers::conductingFluid::setJJsigma(volScalarField& JJsigma)
{
    JJsigma_=JJsigma;
}

void Foam::solvers::conductingFluid::setJxB(volVectorField& JxB)
{   
    JxB_=JxB;
}

Foam::volVectorField& Foam::solvers::conductingFluid::getVelocity()
{   
    return U_;
}

Foam::volScalarField& Foam::solvers::conductingFluid::getPressure()
{   
    return thermo_.p();
}

Foam::volScalarField& Foam::solvers::conductingFluid::getTemperature()
{   
    return thermo_.T();
}

void Foam::solvers::conductingFluid::prePredictor()
{
    isothermalFluid::prePredictor();

    if (pimple.predictTransport())
    {
        thermophysicalTransport->predict();
    }
}


void Foam::solvers::conductingFluid::postCorrector()
{
    isothermalFluid::postCorrector();

    if (pimple.correctTransport())
    {
        thermophysicalTransport->correct();
    }
}


// ************************************************************************* //
