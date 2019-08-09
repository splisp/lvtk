/* 
    Copyright (c) 2019, Michael Fisher <mfisher@kushview.net>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#pragma once

#include <lvtk/feature.hpp>
#include <lv2/instance-access/instance-access.h>

namespace lvtk {

struct InstanceAccess
{
    InstanceAccess() = default;

    /** Get the plugin instance
        @return The plugin instance or nullptr if not available */
    Handle get_instance() const         { return p_plugin_instance; }

    /** Assign the LV2_Handle by LV2 Feature */
    void set_feature (const Feature& feature) {
        if (strcmp (LV2_INSTANCE_ACCESS_URI, feature.URI) == 0)
            p_plugin_instance = reinterpret_cast<Handle> (feature.data);
    }

    private:
        /** @internal Feature Data passed from host */
        Handle p_plugin_instance { nullptr };
};

} /* namespace lvtk */
