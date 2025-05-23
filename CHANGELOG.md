## v0.1: Initially included solvers. ##
* buoyantFoamMHD
* pimpleFoamMHD
* simpleFoamMHD
## v0.2: Added simpleFoam solver that updates electrical potential w/o Elmer. ##
* simpleFoamEpot
## v0.2.1: Added pimpleFoam and buoyantFoam solvers that update electrical potential w/o Elmer. ##
* buoyantFoamEpot
* pimpleFoamEpot
## v0.2.2: Added generateSimpleEOFFiles and generatePimpleEOFFiles for populating case directory with O2E files. ##
* generateSimpleEOFFiles
* generatePimpleEOFFiles
## v0.2.3: Added pimpleFoam and buoyantFoam solvers that updates through Elmer after fixed time interval, but updates only electrical potential for the time steps in-between. ##
* buoyantFoamEpotTransient
* pimpleFoamEpotTransient
## v0.2.4: Fixed generateSimpleEOFFiles. ##
## v0.2.5: Added initializeTemperature for generating initial temperature gradient ##
* initializeTemperature
## v0.3: Added chtMultiRegionFoamEpot and chtMultiRegionFoamEpotTransient solvers for multi region simulations with harmonic Elmer solver. ##
* chtMultiRegionFoamEpot
* chtMultiRegionFoamEpotTransient
## v0.4: Removed duplicates 
* buoyantFoamMHD
* pimpleFoamMHD
* simpleFoamMHD
## Removed EOF file generation tools
* generateSimpleEOFFiles
* generatePimpleEOFFiles
## v1.0: Updated to OpenFOAM-11 compatible solvers
* The new solver foamRunEpot fills the functions of previous solvers buoyantFoamEpot, pimpleFoamEpot and simpleFoamEpot
* The new solver foamRunEpotTransient fills the functions of previous solvers buoyantFoamEpotTransient and pimpleFoamEpotTransient
* The new solver foamMultiRunEpot fills the functions of previous solver chtMultiRegionFoamEpot
* The new solver foamMultiRunEpotTransient fills the functions of previous solver chtMultiRegionFoamEpotTransient
* Added new solver modules conductingFluid, conductingSolid and incompressibleConductingFluid
* Added new solver handler classes conductingRegionSolver and conductingRegionSolvers
* Updated initializeTemperature and initializeMultiRegionTemperature to work with OpenFOAM-11
* Removed deprecated solvers simpleFoamEpot, pimpleFoamEpot, buoyantFoamEpot, pimpleFoamEpotTransient, buoyantFoamEpotTransient, chtMultiRegionFoamEpot and chtMultiRegionFoamEpotTransient
## v1.0.1: Added checkRegionCellZones utility for checking if master thread has any fluid regions with zero cells. ##
* checkRegionCellZones
## v2.0: Added Electromagnetic models ##
* harmonicElectromagneticModel
* transientElectromagneticModel
* electroBase
* coupledElectricPotentialFvPatchScalarField
* coupledCurrentDensityFvPatchVectorField
## v2.1: Added Electromagnetic models ##
* Combined foamRunEpot and foamRunEpotTransient into one solver foamRunEpot
* Combined foamMultiRunEpot and foamMultiRunEpotTransient into one solver foamMultiRunEpot
* Added boundary condition module externalElectricPotentialFvPatchScalarField
## v2.2: Added Electromagnetic models ##
* Added conductingMaterial for regions with electromagnetics, but w/o thermal and fluid dynamics
* Added class feedbackLoopController for controlling voltage input of ElmerFEM conducting region to acheive the required current
* Complemented conductingRegionSolvers and foamMultiRunEpot for setting current conditions on conducting region terminals
* Complemented module conductingRegionSolvers and solver foamMultiRunEpot to store electromagnetic fields for all Elmer regions
## v2.3: Improved Electromagnetic models ##
* Introduced feedback control for pre-set current or voltage in "wire" role regions
## Added new materials ##
* magneticMaterial
## Added new utility initializeRegionSolvers ##
* initializeRegionSolvers
## Removed checkRegionCellZones utility
* checkRegionCellZones

