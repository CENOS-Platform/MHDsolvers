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

#include "electroBase.H"
#include "coupledCurrentDensityFvPatchVectorField.H"
#include "coupledElectricPotentialFvPatchScalarField.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(electroBase, 0);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::electroBase::electroBase
(
    fvMesh& mesh
)
:
    electroPtr_(electromagneticModel::New(mesh)),
    electro_(electroPtr_()),
    electro(electroPtr_)
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::electroBase::~electroBase()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::electroBase::solveElectromagnetics()
{
    //overridden in derived classes
}

void Foam::electroBase::electromagneticPredictor()
{
    electroPtr_->predict();
}

Foam::volVectorField& Foam::electroBase::getJ(bool imaginary)
{
    return electroPtr_->J(imaginary);
}

Foam::volVectorField& Foam::electroBase::getB(bool imaginary)
{
    return electroPtr_->B(imaginary);
}

void Foam::electroBase::initPotE(bool imaginary)
{
    volScalarField& PotE = electroPtr_->PotE(imaginary);
    volScalarField::Boundary& PotEBf = PotE.boundaryFieldRef();
    //Could also cast to mixedFvPatchScalarField,
    //which is the base class of coupledElectricPotentialFvPatchScalarField,
    //but this way it is more clear and excludes other uses of mixedFvPatchScalarField.
    forAll(PotEBf, patchi)
    {
        fvPatchScalarField& pPotE = PotEBf[patchi];
        if (isA<coupledElectricPotentialFvPatchScalarField>(pPotE) )
        {
            coupledElectricPotentialFvPatchScalarField& cpPotE =
            refCast<coupledElectricPotentialFvPatchScalarField>(pPotE);
            cpPotE.evaluate();
        }
    }
}

void Foam::electroBase::initDeltaJ(bool imaginary)
{
        volVectorField& deltaJ = electroPtr_->deltaJ(imaginary);
        volVectorField::Boundary& deltaJBf = deltaJ.boundaryFieldRef();
        forAll(deltaJBf, patchi)
        {
            fvPatchVectorField& pDeltaJ = deltaJBf[patchi];
            if (isA<coupledCurrentDensityFvPatchVectorField>(pDeltaJ) )
            {
                //derived from directionMixedFvPatchVectorField
                coupledCurrentDensityFvPatchVectorField& cpDeltaJ =
                refCast<coupledCurrentDensityFvPatchVectorField>(pDeltaJ);
                //Switch to coupling mode
                cpDeltaJ.initCoupling();
                cpDeltaJ.evaluate();
            }
        }
}

// ************************************************************************* //
