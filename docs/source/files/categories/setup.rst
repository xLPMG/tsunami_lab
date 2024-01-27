.. _setup:

Setup
=================

1. Cloning the repository from GitHub
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: bash

    git clone https://github.com/xLPMG/tsunami_lab.git

2. Initializing & updating submodules
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: bash

    git submodule init
    git submodule update

3. Installing requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To build this project, you will need `SCons <https://scons.org/doc/production/HTML/scons-user/ch01.html>`_
and `GCC <https://gcc.gnu.org/install/>`_.

On MacOS, you can use homebrew for both:

.. code-block:: bash

    brew install scons
    brew install gcc

For I/O, the project requires the ``NetCdf`` library to be installed:

- `MacOS (Homebrew) <https://formulae.brew.sh/formula/netcdf>`_
- Ubuntu: ``sudo apt-get install libnetcdf-dev``
- `Windows <https://downloads.unidata.ucar.edu/netcdf/>`_

The program comes with a gui, for which you will need the `GLFW <https://www.glfw.org/>`_ library.
On MacOS, you can install it easily using `Homebrew. <https://formulae.brew.sh/formula/glfw>`_

You may use the tsunami solver without any gui and thus without the ``GLFW`` library, but you'll have to specify this by later compiling with

.. code-block::

    scons gui=no

4. Building the project
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Navigate to the in step 1 created ``tsunami_lab`` folder and run

.. code:: bash

    scons 

If you wish to build the project for benchmarking without file output, run

.. code:: bash

    scons mode=benchmark

For further run configurations view the SConstruct file at the root of the tsunami_lab directory or checkout the options inside the GUI.

Additional build parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may specify a compiler of your choice, for example ``icpc`` by running

.. code:: bash

    CXX=icpc scons

If you wish to specify an optimization flag, you can do that by adding ``opt=<flag>``, for example:

.. code:: bash

    scons opt=-O2

Currently we support ``-O0``, ``-O1``, ``-O2``, ``-O3`` and ``-Ofast``. The default is ``-O3``.

Generating a report is possible for ``icpc``. To enable it, specify it using

.. code:: bash

    scons report=-qopt-report=<N>

Replace <N> with a number from 1 to 5 for the level of detail. 

5. Building the documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you wish to build the documentation, you will need to install some requirements first.
For the automatic code documentation, `doxygen <https://www.doxygen.nl/download.html>`_ is required.
Furthermore, you will need `Python <https://www.python.org/downloads/>`_. Using Python, you can install
other requirements such as `Sphinx <https://www.sphinx-doc.org/en/master/>`_ and `Breathe <https://www.breathe-doc.org/>`_, 
which can be found in the ``requirements.txt`` file located in the ``docs`` folder. 
They can be installed easily by running

.. code:: bash

    pip install -r requirements.txt

After everything has been installed, you can now run

.. code:: bash

    make html

from inside the ``docs`` directory to build the documentation. 
To access it, open the ``index.html`` file located in ``docs/build/html``.