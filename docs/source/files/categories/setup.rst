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

4. Building the project
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Navigate to the in step 1 created ``tsunami_lab`` folder and run

.. code:: bash

    scons 

for Linux/Windows. If you are on MacOS, use

.. code:: bash

    scons mode="osx"

For further run configurations view the SConstruct file at the root of the tsunami_lab directory.

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