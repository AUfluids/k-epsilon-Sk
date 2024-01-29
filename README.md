<!--  [![Compatibility: OFver](https://img.shields.io/badge/Compatible_with-OpenFOAM.v2112-lightblue.svg)]()  -->
[![Paper: Author](https://img.shields.io/badge/Author-green.svg)](https://sites.google.com/view/zehtabiyan/home)
[![Paper: RENE](https://img.shields.io/badge/Paper-RENE-red.svg)](https://doi.org/10.1016/j.renene.2023.119904)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) 


# Extended-kEpsilon-WindFarmSimulation
An extended $k-\varepsilon$ model integrated with OpenFOAM for simulation of turbine wakes and power losses in wind farms, as proposed by [Zehtabiyan-Rezaie and Abkar (2023)](https://doi.org/10.1016/j.renene.2023.119904) at the Fluid Mechanics and Turbulence research group at Aarhus University, Denmark. 

# Description
We introduce an extended version of the standard $k-\varepsilon$ model within the OpenFOAM framework, with a primary focus on its application in the context of wind-farm simulations. The standard $k-\varepsilon$ model, while widely used, is known to exhibit limitations in accurately representing turbulence characteristics within the wake region of turbines. This often results in an overestimation of turbulence intensity and, consequently, an inaccurate prediction of the power losses in wind farms. To address this challenge, our extended model incorporates an additional term in the turbulent kinetic energy transport equation, which accounts for the influence of turbine forces. This modification is based on a rigorous analytical approach derived from fundamental physical principles. For a comprehensive understanding of this extended turbulence model and its application in wind-farm simulation, additional information can be found in this [publication](https://doi.org/10.1016/j.renene.2023.119904).

# Target platform
The code has been rigorously tested and verified to be fully compatible with OpenFOAM v-2112, ensuring its smooth integration and reliable performance with this specific release.

# Author
[Navid Zehtabiyan-Rezaie](https://sites.google.com/view/zehtabiyan/home)

# How to set the model
1- Download the source code.

2- To enable the momentum-source calculator of the actuator-disk model without rotation (ADM-NR) in OpenFOAM to calculate the source terms based on the disk-averaged velocity, follow these instructions:

$\bullet$ Go to your work directory via the following command:
  
`cd $WM_PROJECT_USER_DIR`
       
$\bullet$ Copy the folder _ADM_NR_diskBased_ to your work directory, and compile the new library with the following command
  
 `wmake`
 
3- Copy the folder _testCase_ to your run directory. To initiate the simulation, adjust number of _numberOfSubdomains_ in _system/decomposeParDict_, and execute the following command:

`.//Allrun`


If you use $ck = 0$ in the _kSource_'s implemented in _fvOptions_ file as _scalarCodedSource_, the additional term in the turbulent kinetic energy equation is deactivated, and the model transforms to the standard $k-\varepsilon$.

4- You will find the results of the calculated velocity, thrust, and power in the disk folders in the _postProcessing_ directory. You can also use the _lineSample_ and _Surfaces_ features in the _system_ directory to extract data over lines and surfaces.

5- Note that for simulation of a new wind farm with different operating conditions of turbines, you need to:

$\bullet$ Update the _system/blockMeshDict_ and _system/topoSetDict_ based on the layout of the wind farm under study.

$\bullet$ Update the _constant/fvOptions_ based on the layout of the wind farm under study and the operating conditions of the turbines.

$\bullet$ Update the the operating conditions of the turbines in _kSource_'s implemented in _fvOptions_ file as _scalarCodedSource_.

# Evaluation of the extended model's performance
Normalized power of turbines in a six-turbine case under full-wake conditions $\theta_w = 270^\circ$, predicted by the standard and extended $k-\varepsilon$ models against large-eddy simulations from the study of [Eidi et al. (2021)](https://doi.org/10.1016/j.renene.2021.08.012). The inter-turbine spacing is 7 rotor diameters and the turbines have a rotor diameter, hub height, and an induction factor of 80 m, 70 m, and 0.25, respectively. The inflow velocity and inflow turbulence intensity at the hub height are 8 m/s and 5.8\%, respectively.

  <img src="https://github.com/nzhtbyn/Extended-kEpsilon-WindFarmSimulation/blob/main/testCase/case1_layout_NP.png" width="900" height="150" alt="Normalized power of turbines in three validation cases">

The performance of the extended $k-\varepsilon$ model has been further assessed through extensive testing against data from wind-tunnel measurements and large-eddy simulations for three validation cases with different layouts under full and partial wake conditions, a $10 \times 3$ array of turbines, and the Horns rev 1 Offshore Wind Farm in the study of [Zehtabiyan-Rezaie and Abkar (2023)](https://doi.org/10.1016/j.renene.2023.119904).

<!-- Streamwise velocity in turbine wakes in a wind farm consisting of a $10 \times 3$ array of turbines. Comparison of profiles against wind-tunnel measurement from the  study of [Chamorro and Porté-Agel](https://doi.org/10.3390/en4111916):
![WindTunnelMeasurements](https://github.com/nzhtbyn/FiguresForCodes/blob/main/Extended-kEpsilon-WindFarmSim/vertical_vs_experiment.png)

Normalized power of the operational wind farm of HR1, predicted through the extended $k-\varepsilon$ model compared with large-eddy simulations data from [Wu and Porté-Agel](https://doi.org/10.1016/j.renene.2014.06.019):
![Hornsrev1](https://github.com/nzhtbyn/FiguresForCodes/blob/main/Extended-kEpsilon-WindFarmSim/NP_HR1.png) -->

# How to cite
Please, cite this library as:
```
@article{ZEHTABIYANREZAIE_RENE2024,
title = {An extended k−ɛ model for wake-flow simulation of wind farms},
journal = {Renewable Energy},
volume = {222},
pages = {119904},
year = {2024},
doi = {https://doi.org/10.1016/j.renene.2023.119904},
url = {https://www.sciencedirect.com/science/article/pii/S0960148123018190},
author = {Navid Zehtabiyan-Rezaie and Mahdi Abkar},
}
```
