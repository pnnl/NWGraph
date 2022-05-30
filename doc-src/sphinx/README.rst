.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

Building documentation
======================


From your terminal shell:

.. code-block: bash

   $ docker run -it -v ${NWMATH_ROOT}:/home/nwmath/NWmath amath583/pages


Then from within docker:

.. code-block: bash

   $ cd NWmath/NWgr/docs/sphinx
   $ bash _scripts/generate_hyde.sh

This will populate (or update) the .md files in the sphinx/libref subdirectory.  The output may be somewhat verbose and is generally safe to ignore.

