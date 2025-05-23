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

#include "conductingSolid.H"
#include "addToRunTimeSelectionTable.H"
#include "findRefCell.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
    defineTypeNameAndDebug(conductingSolid, 0);
    addToRunTimeSelectionTable(solver, conductingSolid, fvMesh);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solvers::conductingSolid::conductingSolid
(
    fvMesh& mesh
)
:
    solid(mesh),

    electroBase(mesh)
{

label PotERefCell = 0;
scalar PotERefValue = 0.0;
setRefCell
( 
    electro.PotE(),
    pimple.dict(),
    PotERefCell,
    PotERefValue
);
mesh.schemes().setFluxRequired(electro.PotE().name());

if (electro.isComplex())
{
    setRefCell
    ( 
        electro.PotE(true),
        pimple.dict(),
        PotERefCell,
        PotERefValue
    );
    mesh.schemes().setFluxRequired(electro.PotE(true).name());
}

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::conductingSolid::~conductingSolid()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //
void Foam::solvers::conductingSolid::thermophysicalPredictor()
{
    volScalarField& e = thermo_.he();
    const volScalarField& rho = thermo_.rho();

    while (pimple.correctNonOrthogonal())
    {
        fvScalarMatrix eEqn
        (
            fvm::ddt(rho, e)
          + thermophysicalTransport->divq(e)
          ==
            fvModels().source(rho, e) + electro.JJsigma()
        );

        eEqn.relax();

        fvConstraints().constrain(eEqn);

        eEqn.solve();

        fvConstraints().constrain(e);

        thermo_.correct();
    }
}

void Foam::solvers::conductingSolid::postCorrector()
{
    if (pimple.correctTransport())
    {
        thermophysicalTransport->correct();
    }
    if (electro.correctElectromagnetics())
    {
        //Correct current density
        electro_.correct();
    }
}
//non-const access for initialization purposes
Foam::volScalarField& Foam::solvers::conductingSolid::getTemperature()
{   
    return thermo_.T();
}

void Foam::solvers::conductingSolid::solveElectromagnetics()
{
    electro_.solve();
}

// ************************************************************************* //
