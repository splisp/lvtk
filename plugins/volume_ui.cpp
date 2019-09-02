
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <lvtk/ui.hpp>
#include <lvtk/ext/ui/idle.hpp>
#include <lvtk/ext/ui/parent.hpp>
#include <lvtk/ext/ui/resize.hpp>
#include <lvtk/ext/log.hpp>

#include "nuklear.hpp"

#define LVTK_VOLUME_UI_URI "http://lvtoolkit.org/plugins/volume/ui"

class VolumeUI : public lvtk::UI<VolumeUI, lvtk::Idle, lvtk::Parent, lvtk::Log,
                                           lvtk::Resize> 
{
public:
    VolumeUI (const lvtk::UIArgs& args) : UI (args) {
        nuke->width      = 208;
        nuke->height     = 85;
        nuke->parent     = (intptr_t) parent();
        nuke->handle     = this;
        nuke->expose     = _expose;

        nuke.init();
        notify_size (nuke.width(), nuke.height());
        nuke.input_begin();
    }

    ~VolumeUI() = default;

    void cleanup()
    {
        nuke.reset();
    }

    LV2UI_Widget widget() {
        return (LV2UI_Widget) nk_pugl_native_window (nuke);
    }

    int idle() {
        nuke.process_events();
        return 0;
    }

    void port_event (uint32_t port, uint32_t size, uint32_t protocol, const void* data) {
        if (port == 4 && protocol == 0) {
            value = *(const float*) data;
        }
    }

    void expose() {
        auto* const ctx = &nuke->ctx;

        if (nk_begin (ctx, "VolumeUI", nk_rect (0, 0, nuke->width, nuke->height), 0))
        {
            nk_layout_row_static (ctx, 30, 180, 1);
            if (nk_slider_float (ctx, -90.f, &value, -24.f, 0.25f))
                write (4, value);
        }
        
        nk_end (ctx);
    }

private:
    nk::pugl nuke;
    float value = 0.f;

    static void _expose (nk_pugl_handle handle) {
        (static_cast<VolumeUI*> (handle))->expose();
    }
};

static lvtk::UIDescriptor<VolumeUI> volume_ui (
    LVTK_VOLUME_UI_URI, {
        LV2_UI__parent
    }
);
