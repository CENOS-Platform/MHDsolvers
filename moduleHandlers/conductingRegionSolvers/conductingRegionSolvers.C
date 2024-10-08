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

#include "conductingRegionSolvers.H"
#include "Time.H"
#include "globalMeshNew.H"
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::conductingRegionSolvers::conductingRegionSolvers(const Time& runTime)
:
    runTime_(runTime)
{

    if (runTime.controlDict().found("regionSolvers"))
    {
        const dictionary& conductingRegionSolversDict =
            runTime.controlDict().subDict("regionSolvers");

        forAllConstIter(dictionary, conductingRegionSolversDict, iter)
        {
            const word regionName(iter().keyword());
            const word solverName(iter().stream());

            names_.append(Pair<word>(regionName, solverName));
        }
    }
    else
    {
        FatalIOErrorInFunction(runTime.controlDict())
            << "regionSolvers list missing from "
            << runTime.controlDict().name()
            << exit(FatalIOError);
    }

    regions_.setSize(names_.size());
    solvers_.setSize(names_.size());
    prefixes_.setSize(names_.size());

    string::size_type nRegionNameChars = 0;

    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        const word& solverName = names_[i].second();
        regionIdx_[regionName] = i;

        // Load the solver library
        solver::load(solverName);

        regions_.set
        (
            i,
            new fvMesh
            (
                IOobject
                (
                    regionName,
                    runTime.name(),
                    runTime,
                    IOobject::MUST_READ
                )
            )
        );

        //Foam::autoPtr<Foam::solver> solverPtr = solver::New(solverName, regions_[i]);
        solvers_.set(i, solver::New(solverName, regions_[i]));

        prefixes_[i] = regionName;
        nRegionNameChars = max(nRegionNameChars, regionName.size());
    }
    //Initialize coupled boundary conditions after all solvers have been loaded,
    //because coupledElectricPotentialFvPatchScalarField constructor requires
    //also the solver of the neighbour patch to be loaded.
    //Initialize PotE before deltaJ BCs, because deltaJ BCs require PotE BCs.
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        bool imaginary = isElectroHarmonic();
        evaluatePotEBfs_(regionName);
        if (imaginary)
        {
            evaluatePotEBfs_(regionName,true);
        }
    }
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        bool imaginary = isElectroHarmonic();
        evaluateDeltaJBfs_(regionName);
        if (imaginary)
        {
            evaluateDeltaJBfs_(regionName,true);
        }
    }

    nRegionNameChars++;

    prefix0_.append(nRegionNameChars, ' ');

    forAll(names_, i)
    {
        prefixes_[i].append(nRegionNameChars - prefixes_[i].size(), ' ');
    }

    //get region characteristic sizes
    characteristicSizes_.setSize(names_.size());
    forAll(names_, i)
    {
        IOdictionary physicalProperties
        (
            IOobject
            (
                "physicalProperties",
                runTime.constant(),
                regions_[i],
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            )
        );
        characteristicSizes_[i] =
        (
            physicalProperties.found("Lchar") ?
            dimensionedScalar("Lchar",dimLength,physicalProperties) :
            dimensionedScalar("Lchar",dimLength,0)
        ).value();
    }

    // get write controls / magnetic field update controls
    if (isElectroHarmonic())
    {
        // Maximum allowable magnetic Reynolds number difference comparing
        // to last magnetic field update.
        // This option controls frequency magnetic field updating is called.
        //     (0,inf) - when relative difference in any cell exceeds given value
        //     0     - every iteration
        maxRemDiff_ = readScalar(runTime.controlDict().lookup("maxRemDiff"));

        // Maximum allowable relative field difference in any cell comparing
        // to last magnetic field update.
        // This option controls frequency magnetic field updating is called.
        //     >1  - once
        //     1 - magnetic Reynolds number is used instead
        //     (0,1) - when relative difference in any cell exceeds given value
        //     0     - every iteration
        maxRelDiff_ = readScalar(runTime.controlDict().lookup("maxRelDiff"));
    }
    else
    {
        writeMultiplier_ = readScalar(runTime.controlDict().lookup("writeMultiplier"));
        writeControlDict_ = word(runTime.controlDict().lookup("writeControl"));
        adjustableRunTime_ = (writeControlDict_=="adjustableRunTime");
    }
    // Get local to global cell id mapping
    int globalCellI = 0;
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        fvMesh& regionMesh = regions_[i];
        const cellList cells = regionMesh.cells();
        forAll(cells, cellI)
        {
            regionToGlobalCellId[std::make_pair(regionName,cellI)] = globalCellI++;
        }
    }
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::conductingRegionSolvers::~conductingRegionSolvers()
{}

// * * * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * * //

//Assigns single fluid/solid region values from global field to region
void Foam::conductingRegionSolvers::scalarGlobalToField_(volScalarField& global,volScalarField& region,const word& regionName)
{
    forAll(region, cellI)
    {
        region[cellI] = global[regionToGlobalCellId[std::make_pair(regionName,cellI)]];
    }
}

void Foam::conductingRegionSolvers::vectorGlobalToField_(volVectorField& global,volVectorField& region,const word& regionName)
{
    forAll(region, cellI)
    {
        region[cellI] = global[regionToGlobalCellId[std::make_pair(regionName,cellI)]];
    }
}

Foam::electroBase* Foam::conductingRegionSolvers::getElectroBasePtr_(const word regionName)
{
    if (isFluid(regionName) || isSolid(regionName))
    {
    Foam::solver* basePtr = solvers_(regionIdx_[regionName]);
        return dynamic_cast<Foam::electroBase*>(basePtr);
    }
    return nullptr;
}

//Note: Could make new solver base class with methods for conducting media
//to avoid casting to each of derived class for accessing new common methods.
Foam::solvers::conductingFluid* Foam::conductingRegionSolvers::getFluidPtr_(const word regionName)
{
    if (isFluid(regionName))
    {
        Foam::solver* basePtr = solvers_(regionIdx_[regionName]);
        return dynamic_cast<Foam::solvers::conductingFluid*>(basePtr);
    }
    return nullptr;
}

Foam::solvers::conductingSolid* Foam::conductingRegionSolvers::getSolidPtr_(const word regionName)
{
    if (isSolid(regionName))
    {
        Foam::solver* basePtr = solvers_(regionIdx_[regionName]);
        return dynamic_cast<Foam::solvers::conductingSolid*>(basePtr);
    }
    return nullptr;
}
//initializes boundary conditions
void Foam::conductingRegionSolvers::evaluatePotEBfs_(const word regionName, bool imaginary)
{
    if (getElectroBasePtr_(regionName))
    {
        getElectroBasePtr_(regionName)->initPotE(imaginary);
    }
}
//initializes boundary conditions
void Foam::conductingRegionSolvers::evaluateDeltaJBfs_(const word regionName, bool imaginary)
{
    if (getElectroBasePtr_(regionName))
    {
        getElectroBasePtr_(regionName)->initDeltaJ(imaginary);
    }
}

// * * * * * * * * * * * * * * * Public Member Functions  * * * * * * * * * * * * * //

void Foam::conductingRegionSolvers::setGlobalPrefix() const
{
    Sout.prefix() = prefix0_;
}

void Foam::conductingRegionSolvers::setPrefix(const label i) const
{
    Sout.prefix() = prefixes_[i];
}

void Foam::conductingRegionSolvers::resetPrefix() const
{
    Sout.prefix() = string::null;
}

Foam::fvMesh& Foam::conductingRegionSolvers::mesh(const word regionName)
{
    return regions_[regionIdx_[regionName]];
}

void Foam::conductingRegionSolvers::solveElectromagnetics(const word regionName)
{
    if (getElectroBasePtr_(regionName))
    {
        getElectroBasePtr_(regionName)->solveElectromagnetics();
    }
}

void Foam::conductingRegionSolvers::electromagneticPredictor(const word regionName)
{
    if (getElectroBasePtr_(regionName))
    {
        getElectroBasePtr_(regionName)->electromagneticPredictor();
    }
}
//Returns global mesh
const Foam::fvMesh& Foam::conductingRegionSolvers::globalMesh()
{
    // If pointer empty, create new global mesh.
    if (globalMesh_.empty())
    {
        globalMesh_ = globalMeshNew_();
    }
    // Check if global mesh is already created.
    if (globalMesh_.valid())
    {
        return *globalMesh_;
    }
    else
    {
        FatalIOError << "Failed to get global mesh!\n"
        << exit(FatalIOError);
    }
}
//Assigns fluid and solid region values from global to each region field
void Foam::conductingRegionSolvers::setJ(volVectorField& globalField, bool imaginary)
{
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        if (getElectroBasePtr_(regionName))
        {
            volVectorField& regionField = getElectroBasePtr_(regionName)->getJ(imaginary);
            vectorGlobalToField_(globalField,regionField,regionName);
            regionField.correctBoundaryConditions();
        }
    }
}
//Assigns fluid and solid region values from global to each region field
void Foam::conductingRegionSolvers::setB(volVectorField& globalField, bool imaginary)
{
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        if (getElectroBasePtr_(regionName))
        {
            volVectorField& regionField = getElectroBasePtr_(regionName)->getB(imaginary);
            vectorGlobalToField_(globalField,regionField,regionName);
            regionField.correctBoundaryConditions();
        }
    }
}
//Assigns U_old = U
void Foam::conductingRegionSolvers::storeU(const word regionName)
{
    if (getFluidPtr_(regionName))
    {
        getFluidPtr_(regionName)->storeU();
    }
}
//returns read-only access to electro module
const Foam::electromagneticModel& Foam::conductingRegionSolvers::getElectro(const word regionName)
{
    if (getElectroBasePtr_(regionName))
    {
        return getElectroBasePtr_(regionName)->electro;
    }
    else
    {
        FatalIOError << " electroBase class not found for region "
        << regionName << "!\n" << "Cannot get electromagnetic model!\n" << exit(FatalIOError);
    }
}
const Foam::solvers::conductingFluid& Foam::conductingRegionSolvers::getFluid(const word regionName)
{
    if (getFluidPtr_(regionName))
    {
        const Foam::solvers::conductingFluid& fluidRef(*getFluidPtr_(regionName));
        return fluidRef;
    }
    else
    {
        FatalIOError << " region " << regionName << " solver is not "
        << fluidSolverName_ << "!\n" << "Cannot get fluid!\n" << exit(FatalIOError);
    }
}
const Foam::solvers::conductingSolid& Foam::conductingRegionSolvers::getSolid(const word regionName)
{
    if (getSolidPtr_(regionName))
    {
        const Foam::solvers::conductingSolid& solidRef(*getSolidPtr_(regionName));
        return solidRef;
    }
    else
    {
        FatalIOError << " region " << regionName << " solver is not "
        << solidSolverName_ << "!\n" << "Cannot get solid!\n" << exit(FatalIOError);
    }
}

//Assigns single fluid/solid region values from region to global field
void Foam::conductingRegionSolvers::scalarFieldToGlobal(volScalarField& global,const volScalarField& region,const word& regionName)
{
    forAll(region, cellI)
    {
        global[regionToGlobalCellId[std::make_pair(regionName,cellI)]] = region[cellI];
    }
}

void Foam::conductingRegionSolvers::vectorFieldToGlobal(volVectorField& global,const volVectorField& region,const word& regionName)
{
    forAll(region, cellI)
    {
        global[regionToGlobalCellId[std::make_pair(regionName,cellI)]] = region[cellI];
    }
}
bool Foam::conductingRegionSolvers::isFluid(const word regionName)
{
    return names_[regionIdx_[regionName]].second() == fluidSolverName_;
}

bool Foam::conductingRegionSolvers::isSolid(const word regionName)
{
    return names_[regionIdx_[regionName]].second() == solidSolverName_;
}

void Foam::conductingRegionSolvers::setPotentialCorrectors(const dictionary& dict)
{
    nPotEcorr_ = dict.lookupOrDefault<label>("nPotECorrectors", nPotEcorr_);
}

bool Foam::conductingRegionSolvers::correctElectroPotential()
{
    if (PotEcorr_ >= nPotEcorr_)
    {
        PotEcorr_ = 0;
        return false;
    }

    PotEcorr_++;

    return true;
}

bool Foam::conductingRegionSolvers::isElectroHarmonic()
{
    forAll(names_, i)
    {
        const word& regionName = names_[i].first();
        if (isFluid(regionName) || isSolid(regionName))
        {
            return getElectro(regionName).isComplex();
        }
    }
    return false;
}

bool Foam::conductingRegionSolvers::updateMagneticField()
{
    bool doUpdate = false;
    if (isElectroHarmonic())
    {
        scalar maxRemDiff_local = SMALL;        
        scalar maxRelDiff_local = SMALL;

        forAll(names_, i)
        {
            const word& regionName = names_[i].first();
            if (isFluid(regionName))
            {
                const volVectorField& U = getFluid(regionName).U;
                const volVectorField& U_old = getFluid(regionName).U_old;
                maxRemDiff_local = max(
                    mu_0 * characteristicSizes_[i] *
                    max(getElectro(regionName).sigmaInv()*mag(U_old-U)).value(),
                    maxRemDiff_local);        

                maxRelDiff_local = max(
                    (max(mag(U_old-U)/(average(mag(U))+smallU))).value(),
                    maxRemDiff_local);
            }
        }

        if((maxRelDiff_local>maxRelDiff_ || maxRelDiff_<SMALL) && maxRelDiff_+SMALL<=1.0) {
            doUpdate = true;
        }
        else if(maxRemDiff_local>maxRemDiff_ && maxRelDiff_-SMALL<=1.0) {
            doUpdate = true;
        }
    }
    else
    {
        if (adjustableRunTime_)
        {
            if (runTime_.writeTime()) doUpdate = true;
        }
        else
        {
            writeCounter_++;
            if ( (writeCounter_ % writeMultiplier_) == 0 && runTime_.run()) doUpdate = true;
        }
    }
    return doUpdate;
}

void Foam::conductingRegionSolvers::calcTemperatureGradient(const word regionName)
{
    IOdictionary physicalProperties
    (
        IOobject
        (
            "physicalProperties",
            runTime_.constant(),
            mesh(regionName),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );
    if
    ( 
        physicalProperties.found("temperature_multiplier") &&
        physicalProperties.found("temperature_addition")
    )
    {
        dimensionedVector temperature_multiplier
        (
            "temperature_multiplier",
            dimTemperature/dimLength,
            physicalProperties
        );
        dimensionedScalar temperature_addition
        (
            "temperature_addition",
            dimTemperature,
            physicalProperties
        );

        if (getSolidPtr_(regionName))
        {
            volScalarField& T = getSolidPtr_(regionName)->getTemperature();
            T = (mesh(regionName).C() & temperature_multiplier) +  temperature_addition;
            T.write();
        }
        else if (getFluidPtr_(regionName))
        {
            volScalarField& T = getFluidPtr_(regionName)->getTemperature();
            T = (mesh(regionName).C() & temperature_multiplier) +  temperature_addition;
            T.write();
        }
        /*
        // Display warning message if solver is not fluid or solid
        else
        {
            Info << "Warning: region " << regionName << " solver is not " << fluidSolverName_
            << " or " << solidSolverName_ << "!\n" << "Cannot set temperature gradient!\n"; 
        }
        */
    }
}

Foam::List<Foam::Pair<Foam::word>> Foam::conductingRegionSolvers::getNames()
{
    return names_;
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

Foam::solver& Foam::conductingRegionSolvers::operator[](const label i)
{
    setPrefix(i);
    return solvers_[i];
}

Foam::solver& Foam::conductingRegionSolvers::operator()(const word regionName)
{
    setPrefix(regionIdx_[regionName]);
    return solvers_[regionIdx_[regionName]];
}
/*
Foam::solver* Foam::conductingRegionSolvers::operator()(const label i)
{
    return solvers_(i);
}*/


// ************************************************************************* //
