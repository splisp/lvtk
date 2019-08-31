
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "../libs/nuklear/nuklear.h"

#define NK_PUGL_IMPLEMENTATION
#define NK_PUGL_LOAD_OPENGL_EXTENSIONS
#include "../libs/nuklear/nuklear_pugl.h"

#include <lvtk/ui.hpp>

#define LVTK_VOLUME_UI_URI "http://lvtoolkit.org/plugins/volume/ui"

class VolumeUI : public lvtk::UI<VolumeUI> {
public:
    VolumeUI (const lvtk::UIArgs& args) : UI (args) { 
        nuke.width = 640;
        nuke.height = 360;
        nk_pugl_init (&nuke);
        nk_input_begin (&nuke.ctx);
    }

    ~VolumeUI() = default;

    void cleanup() {
        nk_input_end (&nuke.ctx);
        nk_pugl_destroy (&nuke);
    }

    LV2UI_Widget get_widget() {
        return (LV2UI_Widget) puglGetNativeWindow (nuke.view);
    }

    int idle() {
        nk_pugl_process_events (&nuke);
        return 0;
    }

private:
    nk_pugl nuke;
};

static lvtk::UIDescriptor<VolumeUI> volume_ui ("LVTK_VOLUME_UI_URI");
