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

#include <lv2/ui/ui.h>
#include <lvtk/ext/extension.hpp>

namespace lvtk {

/** Support for UI Resizing
    @headerfile lvtk/ext/ui/resize.hpp
    @ingroup ui
*/
template<class I> 
struct Resize : Extension<I>
{
    /** @private */
    Resize (const FeatureList& features) {
        if (auto* data = features.data (LV2_UI__resize))
            resize = (LV2UI_Resize*) data;
    }
    
    /** Called from the <em>plugin</em> to notify the host of size change
        @returns non-zero on error
     */
    int notify_size (int width, int height) {
        return (resize != nullptr) ? resize->ui_resize (resize->handle, width, height)
                                   : 1;
    }

    /** Called by the <em>host</em> to request a new UI size
        @return non-zero on error
     */
    int size_requested (int width, int height) { return 0; }

protected:
    /** @private */
    static void map_extension_data (ExtensionMap& emap) {
        static LV2UI_Resize _resize = { nullptr, _ui_resize };
        emap[LV2_UI__resize] = &_resize;
    }

private:
    LV2UI_Resize* resize = nullptr;
    inline static int _ui_resize (LV2UI_Feature_Handle handle, int width, int height) {
        return (static_cast<I*>(handle))->size_requested (width, height);
    }
};

}
