/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2023 OpenFOAM Foundation
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
    Foam::coupledCurrentDensityFvPatchVectorField

Description

    Specifies gradient and temperature such that the equations are the same
    on both sides:
      - valueFraction = 1 -> fixed value
      - refValue = sigma * ( -grad(PotE) + UxB ),

    where sigma is the electric conductivity of the region, PotE is the
    electric potential and UxB is the velocity contribution from the region.

SourceFiles
    coupledCurrentDensityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef coupledCurrentDensityFvPatchVectorField_H
#define coupledCurrentDensityFvPatchVectorField_H

#include "directionMixedFvPatchFields.H"
//#include "coupledElectricPotentialFvPatchScalarField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
            Class coupledCurrentDensityFvPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class coupledCurrentDensityFvPatchVectorField
:
    public directionMixedFvPatchVectorField
{
    // Private Data

        //- Default name of field on this region
        const word deltaJname_ = "deltaJ";
        const word Jname_ = "J";

        //- Default name of electric potential
        const word ePotName_ = "PotE";

        //- Default name of velocity contribution
        const word UxBname_ = "deltaUxB";

        //- Direction of current
        //- Flips direction for source interfaces
        int direction_ = 1;

        //- Coupled flag
        bool coupled_ = false;


protected:

    //- Get the patch sigma
    virtual void getValues
    (
        tmp<scalarField>& sigma
    ) const;

    word suffix() const;

    bool isTerminal(word) const;

public:

    //- Runtime type information
    TypeName("coupledCurrentDensity");


    // Constructors

        //- Construct as copy setting internal field reference
        coupledCurrentDensityFvPatchVectorField
        (
            const coupledCurrentDensityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );
        //- Construct from patch, internal field and dictionary
        coupledCurrentDensityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  coupledCurrentDensityFvPatchVectorField onto a
        //  new patch
        coupledCurrentDensityFvPatchVectorField
        (
            const coupledCurrentDensityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new coupledCurrentDensityFvPatchVectorField
                (
                    *this,
                    iF
                )
            );
        }


    // Member Functions

        //- One way switch to coupled boundary condition
        //- Uncoupled interfaces will act as walls
        virtual void initCoupling();

        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
