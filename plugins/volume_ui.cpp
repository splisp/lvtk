
#include <iostream>
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
#include <lvtk/ext/ui/parent.hpp>
#include <lvtk/ext/log.hpp>

#define LVTK_VOLUME_UI_URI "http://lvtoolkit.org/plugins/volume/ui"

class VolumeUI : public lvtk::UI<VolumeUI, lvtk::Idle, lvtk::Parent, lvtk::Log> {
public:
    VolumeUI (const lvtk::UIArgs& args) : UI (args) { 
        memset (&nuke, 0, sizeof(nuke));
        nuke.width      = 640;
        nuke.height     = 360;
        nuke.handle     = this;
        nuke.expose     = _expose;
        nk_pugl_init (&nuke);

        if (parent) {
            log << "got a parent\n";
            puglInitWindowParent (nuke.view, (PuglNativeWindow) parent());
            puglCreateWindow (nuke.view, "");
            {
                // struct nk_font_atlas *atlas = &app.atlas;
                nk_pugl_font_stash_begin (&nuke);
                /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
                /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
                /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
                /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
                /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
                /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
                nk_pugl_font_stash_end (&nuke);
                /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
                /*nk_style_set_font(ctx, &droid->handle);*/
            }
        }
        
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
        // std::clog << "idle()" << std::endl;
        return 0;
    }

    void expose (struct nk_context* ctx) {
        std::clog << "expose()" << std::endl;
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

static lvtk::UIDescriptor<VolumeUI> volume_ui (LVTK_VOLUME_UI_URI, {
    // LV2_UI__idleInterface
});
