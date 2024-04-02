#include "VRViewer.h"

#include <memory>

#include "OXRContext.h"


void VRViewer::run() {
    context = std::make_shared<OXRContext>();
    context->setup();
}

