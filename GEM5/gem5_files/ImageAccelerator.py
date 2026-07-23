# ImageAccelerator.py
# Registra el acelerador como dispositivo GEM5.
# Ubicacion: ~/gem5/src/dev/ImageAccelerator.py

from m5.params import *
from m5.proxy  import *
from m5.SimObject import SimObject

class ImageAccelerator(SimObject):
    type       = 'ImageAccelerator'
    cxx_header = "dev/image_accelerator_gem5.hh"
    cxx_class  = 'gem5::ImageAcceleratorGem5'

    # Puerto hacia el bus del sistema
    cpu_side = ResponsePort("CPU side port")
