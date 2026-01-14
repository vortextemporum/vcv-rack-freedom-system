#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    // Register modules
    p->addModel(modelGenesisMono);
    p->addModel(modelGenesisPoly);
}
