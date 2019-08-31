
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
#include <lvtk/ext/ui/idle.hpp>

#define LVTK_VOLUME_UI_URI "http://lvtoolkit.org/plugins/volume/ui"

class VolumeUI : public lvtk::UI<VolumeUI, lvtk::Idle> {
public:
    VolumeUI (const lvtk::UIArgs& args) : UI (args) { 
        memset (&nuke, 0, sizeof(nuke));
        nuke.width      = 640;
        nuke.height     = 360;
        nuke.handle     = this;
        nuke.expose     = _expose;
        nk_pugl_init (&nuke);
        nk_input_begin (&nuke.ctx);
    }

    ~VolumeUI() = default;

    void cleanup() {
        nk_input_end (&nuke.ctx);
        nk_pugl_destroy (&nuke);
        memset (&nuke, 0, sizeof(nuke));
    }

    LV2UI_Widget get_widget() {
        return (LV2UI_Widget) puglGetNativeWindow (nuke.view);
    }

    int idle() {
        nk_pugl_process_events (&nuke);
        return 0;
    }

    void expose (struct nk_context* ctx) {
        if (nk_begin (ctx, "VolumeUI", nk_rect (0, 0, nuke.width, nuke.height), 0))
        {   
            nk_layout_row_static (ctx, 30, 80, 1);
            if (nk_button_label (ctx, "Button"))
                fprintf (stdout, "Button Pressed\n");
        }
        
        nk_end (ctx);
    }

private:
    nk_pugl nuke;
    static void _expose (pugl_handle_t handle, struct nk_context* ctx) {
        (static_cast<VolumeUI*> (handle))->expose (ctx);
    }
};

static lvtk::UIDescriptor<VolumeUI> volume_ui ("LVTK_VOLUME_UI_URI", {
    LV2_UI__idleInterface
});
