/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2020 ENERCON GmbH
    Copyright (C) 2018-2020 OpenCFD Ltd
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

#include "ADM_NR_diskBased.H"
#include "geometricOneField.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{
    defineTypeNameAndDebug(ADM_NR_diskBased, 0);
    addToRunTimeSelectionTable(option, ADM_NR_diskBased, dictionary);
}
}


const Foam::Enum
<
    Foam::fv::ADM_NR_diskBased::forceMethodType
>
Foam::fv::ADM_NR_diskBased::forceMethodTypeNames
({
    { forceMethodType::FROUDE, "Froude" },
    { forceMethodType::VARIABLE_SCALING, "variableScaling" },
});


const Foam::Enum
<
    Foam::fv::ADM_NR_diskBased::monitorMethodType
>
Foam::fv::ADM_NR_diskBased::monitorMethodTypeNames
({
    { monitorMethodType::POINTS, "points" },
    { monitorMethodType::CELLSET, "cellSet" },
});


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::fv::ADM_NR_diskBased::writeFileHeader(Ostream& os)
{
    writeFile::writeHeader(os, "Actuation disk source based ib disk-averaged velocities");
    writeFile::writeCommented(os, "Time");
    writeFile::writeCommented(os, "Uref");
    writeFile::writeCommented(os, "Cp");
    writeFile::writeCommented(os, "Ct");

    if (forceMethod_ == forceMethodType::FROUDE)
    {
        writeFile::writeCommented(os, "a");
        writeFile::writeCommented(os, "T");
    }
    else if (forceMethod_ == forceMethodType::VARIABLE_SCALING)
    {
        writeFile::writeCommented(os, "Udisk");
        writeFile::writeCommented(os, "CpStar");
        writeFile::writeCommented(os, "CtStar");
        writeFile::writeCommented(os, "T");
        writeFile::writeCommented(os, "P");
    }

    os  << endl;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::fv::ADM_NR_diskBased::setMonitorCells(const dictionary& dict)
{
    switch (monitorMethod_)
    {
        case monitorMethodType::POINTS:
        {
            Info<< "    - selecting cells using points" << endl;

            labelHashSet selectedCells;

            List<point> monitorPoints;

            const dictionary* coeffsDictPtr = dict.findDict("monitorCoeffs");
            if (coeffsDictPtr)
            {
                coeffsDictPtr->readIfPresent("points", monitorPoints);
            }
            else
            {
                monitorPoints.resize(1);
                dict.readEntry("upstreamPoint", monitorPoints.first());
            }

            for (const auto& monitorPoint : monitorPoints)
            {
                const label celli = mesh_.findCell(monitorPoint);
                if (celli >= 0)
                {
                    selectedCells.insert(celli);
                }

                const label globalCelli = returnReduce(celli, maxOp<label>());
                if (globalCelli < 0)
                {
                    WarningInFunction
                        << "Unable to find owner cell for point "
                        << monitorPoint << endl;
                }
            }

            monitorCells_ = selectedCells.sortedToc();
            break;
        }
        case monitorMethodType::CELLSET:
        {
            Info<< "    - selecting cells using cellSet "
                << cellSetName_ << endl;

            monitorCells_ = cellSet(mesh_, cellSetName_).sortedToc();
            break;
        }
        default:
        {
            FatalErrorInFunction
                << "Unknown type for monitoring of incoming velocity"
                << monitorMethodTypeNames[monitorMethod_]
                << ". Valid monitor method types : "
                << monitorMethodTypeNames
                << exit(FatalError);
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::ADM_NR_diskBased::ADM_NR_diskBased
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    cellSetOption(name, modelType, dict, mesh),
    writeFile(mesh, name, modelType, coeffs_),
    forceMethod_
    (
        forceMethodTypeNames.getOrDefault
        (
            "variant",
            coeffs_,
            forceMethodType::FROUDE
        )
    ),
    monitorMethod_
    (
        monitorMethodTypeNames.getOrDefault
        (
            "monitorMethod",
            coeffs_,
            monitorMethodType::POINTS
        )
    ),
    sink_
    (
        coeffs_.getOrDefault<bool>("sink", true)
      ? 1
      : -1
    ),
    writeFileStart_(coeffs_.getOrDefault<scalar>("writeFileStart", 0)),
    writeFileEnd_(coeffs_.getOrDefault<scalar>("writeFileEnd", VGREAT)),
    diskArea_
    (
        coeffs_.getCheck<scalar>
        (
            "diskArea",
            scalarMinMax::ge(VSMALL)
        )
    ),
    diskDir_
    (
        coeffs_.getCheck<vector>
        (
            "diskDir",
            [&](const vector& vec){ return mag(vec) > VSMALL; }
        ).normalise()
    ),
    UvsCpPtr_(Function1<scalar>::New("Cp", coeffs_)),
    UvsCtPtr_(Function1<scalar>::New("Ct", coeffs_)),
    monitorCells_()
{
    setMonitorCells(coeffs_);

    fieldNames_.setSize(1, "U");

    applied_.setSize(fieldNames_.size(), false);

    Info<< "    - creating actuation disk zone: " << this->name() << endl;

    Info<< "    - force computation method: "
        << forceMethodTypeNames[forceMethod_] << endl;

    writeFileHeader(file());
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fv::ADM_NR_diskBased::addSup
(
    fvMatrix<vector>& eqn,
    const label fieldi
)
{
    if (V() > VSMALL)
    {
        calc(geometricOneField(), geometricOneField(), eqn);
    }
}


void Foam::fv::ADM_NR_diskBased::addSup
(
    const volScalarField& rho,
    fvMatrix<vector>& eqn,
    const label fieldi
)
{
    if (V() > VSMALL)
    {
        calc(geometricOneField(), rho, eqn);
    }
}


void Foam::fv::ADM_NR_diskBased::addSup
(
    const volScalarField& alpha,
    const volScalarField& rho,
    fvMatrix<vector>& eqn,
    const label fieldi
)
{
    if (V() > VSMALL)
    {
        calc(alpha, rho, eqn);
    }
}


bool Foam::fv::ADM_NR_diskBased::read(const dictionary& dict)
{
    if (cellSetOption::read(dict) && writeFile::read(dict))
    {
        dict.readIfPresent("sink", sink_);
        dict.readIfPresent("writeFileStart", writeFileStart_);
        dict.readIfPresent("writeFileEnd", writeFileEnd_);
        dict.readIfPresent("diskArea", diskArea_);
        if (diskArea_ < VSMALL)
        {
            FatalErrorInFunction
                << "Actuator disk has zero area: "
                << "diskArea = " << diskArea_
                << exit(FatalIOError);
        }

        dict.readIfPresent("diskDir", diskDir_);
        diskDir_.normalise();
        if (mag(diskDir_) < VSMALL)
        {
            FatalErrorInFunction
                << "Actuator disk surface-normal vector is zero: "
                << "diskDir = " << diskDir_
                << exit(FatalIOError);
        }

        return true;
    }

    return false;
}


// ************************************************************************* //
