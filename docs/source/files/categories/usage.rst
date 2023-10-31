.. _usage:

Usage
======

Navigate to the local git repository. If the name wasnt explicitly changed, this should be the `tsunami_lab` folder.
If you have yet to set up and build the git repository locally, check out the :ref:`setup` page first.


Run the code
-----------------

Navigate to the `build` folder which was automatically created during the setup process.
To run the project, execute

.. code::
    
    ./tsunami_lab <x_cells> <solver>

from the command line. 
You have to specify the amount of cells in the x direction and optionally which solver you would like to use.
The `<x_cells>` parameter allows a size_t number which will be rounded down to the nearest integer.
`<solver>` can either be `roe` or `fwave`. If you dont specify a solver, roe will be used by default.

Valid queries can look like this:

.. code::
    
    ./tsunami_lab 12.000001
    ./tsunami_lab 12 fwave
    ./tsunami_lab 12.9999999 roe

Note that for all of those queries, `x_cells` will be rounded to 12.
