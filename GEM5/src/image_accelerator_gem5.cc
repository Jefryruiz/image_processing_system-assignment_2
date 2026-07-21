/**
 * image_accelerator_gem5.cc
 *
 * Implementacion del wrapper GEM5 para el acelerador.
 * Ubicacion en GEM5: ~/gem5/src/dev/image_accelerator_gem5.cc
 */

#include "dev/image_accelerator_gem5.hh"

namespace gem5 {

// GEM5 necesita este metodo para instanciar el objeto desde Python
ImageAcceleratorGem5 *
ImageAcceleratorParams::create() const
{
    return new ImageAcceleratorGem5(*this);
}

} // namespace gem5
