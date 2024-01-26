What is the tsunami lab?
#########################

The `tsunami lab <https://scalable.uni-jena.de/opt/tsunami/>`_ is a course taught at the `University of Jena <https://www.uni-jena.de/en/universityofjena>`_ as 
the practical lab for the lecture on parallel computing.
The goal is to write a C++ tsunami `wave equation <https://en.wikipedia.org/wiki/Wave_equation>`_ `solver <https://www.clawpack.org/riemann_book/html/Index.html>`_, optimize and parallelize it and run tsunami simulations on an `HPC cluster <https://www.hpe.com/asia_pac/en/what-is/hpc-clusters.html>`_.

During the class, we received weekly assignments which make up the final grade. Our (`Luca-Philipp Grumbach <https://github.com/xLPMG>`_ and `Richard Hofmann <https://github.com/ZeyxRew>`_)
implementations of the given weekly tasks are what make up this repository - a fully functional 2D wave equation solver for Linux and MacOS.

What does the code do?
##########################

The solver can be used either with pre-defined setups or with custom `bathymetry <https://en.wikipedia.org/wiki/Bathymetry>`_ & `displacement <https://en.wikipedia.org/wiki/Displacement_(fluid)>`_ files.
A bathymetry file contains info on the depth of water, while the displacement file describes the effects on the water level due to e.g. an earthquake.

Using this data, our program will numerically calculate (if specified in parallel on multiple CPU cores) the wave propagation after the initial input data, that is specifically
calculate and output how a tsunami forms and moves after an earthquake. 

The solver outputs this data either in form of a ``.csv`` file or a ``netcdf`` file (recommended for large data).
This data can then be visualized, for example using `Paraview <https://www.paraview.org/>`_:

Documentation structure
#########################

This documentation is divided into different categories categories:

* **First steps**
    * **Introduction** - provides a broad overview
    * **Setup** - requirements and explanations on how to put this repository into action
    * **Usage** - information on how to excecute this project and its tests
* **Code documentation**
    * **Assignments** - divided into the different weekly steps, this category provides a detailed and manually written documentation of the source code
    * **Namespaces** -  automatically created code documentation using doxygen, divided into the namespaces

