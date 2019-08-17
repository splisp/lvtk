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

#include <lvtk/ext/ui.hpp>
#include <lvtk/interface/interface.hpp>

namespace lvtk {

/** Adds idle interface to your UI instance
    @ingroup uinterfaces
*/
template<class I>
struct Idle : Interface<I>
{
    /** @private */
    Idle (const FeatureList&) { }

    /** Called repeatedly by the host to drive your UI.  Return non-zero
        to stop receiving callbacks.
    */
    inline int idle() { return 0; }

protected:
    inline static void map_extension_data (ExtensionMap& dmap) {
        static const LV2UI_Idle_Interface _idle_iface =  { _idle };
        dmap[LV2_UI__idleInterface] = &_idle_iface;
    }

private:
    static int _idle (LV2UI_Handle ui) { return (static_cast<I*> (ui))->idle(); }
};

}
