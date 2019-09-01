
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
#include <lvtk/ext/ui/resize.hpp>
#include <lvtk/ext/log.hpp>

#define LVTK_VOLUME_UI_URI "http://lvtoolkit.org/plugins/volume/ui"

class VolumeUI : public lvtk::UI<VolumeUI, lvtk::Idle, lvtk::Parent, lvtk::Log,
                                           lvtk::Resize> 
{
public:
    VolumeUI (const lvtk::UIArgs& args) : UI (args) { 
        memset (&nuke, 0, sizeof (nuke));
        nuke.width      = 208;
        nuke.height     = 85;
        nuke.parent     = (intptr_t) parent();
        nuke.handle     = this;
        nuke.expose     = _expose;
        nk_pugl_init (&nuke);
        notify_size (nuke.width, nuke.height);
        nk_input_begin (&nuke.ctx);
    }

    ~VolumeUI() = default;

    void cleanup()
    {
        nk_input_end (&nuke.ctx);
        nk_pugl_destroy (&nuke);
        memset (&nuke, 0, sizeof(nuke));
    }

    LV2UI_Widget widget() {
        return (LV2UI_Widget) nk_pugl_native_window (&nuke);
    }

    int idle() {
        nk_pugl_process_events (&nuke);
        return 0;
    }

    void port_event (uint32_t port, uint32_t size, uint32_t protocol, const void* data) {
        if (port == 4 && protocol == 0) {
            value = *(const float*) data;
        }
    }

    void expose (struct nk_context* ctx) {
        if (nk_begin (ctx, "VolumeUI", nk_rect (0, 0, nuke.width, nuke.height), 0))
        {
            nk_layout_row_static (ctx, 30, 180, 1);
            if (nk_slider_float (ctx, -90.f, &value, -24.f, 0.25f))
                write (4, value);
        }
        
        nk_end (ctx);
    }

private:
    nk_pugl nuke;
    float value = 0.f;

    static void _expose (nk_pugl_handle handle, struct nk_context* ctx) {
        (static_cast<VolumeUI*> (handle))->expose (ctx);
    }
};

static lvtk::UIDescriptor<VolumeUI> volume_ui (
    LVTK_VOLUME_UI_URI, {
        LV2_UI__parent
    }
);
