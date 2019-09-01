/*
 * Nuklear - v1.17 - public domain
 * no warrenty implied; use at your own risk.
 * copied and modifed from nuklear_xlib_gl2
 *  - authored from 2015-2016 by Micha Mettke
 *  - modified from 2019 for Pugl by Michael Fisher
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_PUGL_GL_H_
#define NK_PUGL_GL_H_

#include "pugl/gl.h"
#include "pugl/pugl.h"
#include "pugl/pugl_gl_backend.h"

#ifdef _cplusplus
  extern "C" {
#endif

typedef void* pugl_handle_t;

typedef struct nk_pugl_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
} pugl_vertex_t;

typedef struct nk_pugl_device {
    struct nk_buffer cmds;
    struct nk_buffer ebuf;
    struct nk_buffer vbuf;
    struct nk_draw_null_texture null;
    GLuint font_tex;
} pugl_device_t;

typedef struct _nk_pugl {
    struct nk_context ctx;
    pugl_device_t dev;
    struct nk_font_atlas atlas;
    
    long last_button_click;

    PuglView* view;
    int width;
    int height;

    pugl_handle_t handle;
    void (*close)(pugl_handle_t);
    void (*expose)(pugl_handle_t, struct nk_context*);
} nk_pugl;

NK_API void     nk_pugl_init (nk_pugl*);
NK_API void     nk_pugl_destroy (nk_pugl*);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */

#ifdef NK_PUGL_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stddef.h>

#ifndef NK_PUGL_DOUBLE_CLICK_LO
 #define NK_PUGL_DOUBLE_CLICK_LO 20
#endif
#ifndef NK_PUGL_DOUBLE_CLICK_HI
 #define NK_PUGL_DOUBLE_CLICK_HI 200
#endif

NK_INTERN long
nk_timestamp(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (long)((long)tv.tv_sec * 1000 + (long)tv.tv_usec/1000);
}

NK_INTERN void
nk_pugl_device_upload_atlas (nk_pugl* pugl, const void *image, int width, int height)
{
    pugl_device_t *dev = &pugl->dev;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_INTERN void
nk_pugl_render_begin(int32_t width, int32_t height)
{
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable (GL_CULL_FACE);
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_SCISSOR_TEST);
    glEnable (GL_BLEND);
    glEnable (GL_TEXTURE_2D);

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)width,(GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

NK_INTERN void
nk_pugl_render(PuglView* view, enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    nk_pugl* pugl = (nk_pugl*) puglGetHandle (view);
    pugl_device_t *dev = &pugl->dev;

    glClear (GL_COLOR_BUFFER_BIT);
    glClearColor (0.1f, 0.18f, 0.24f, 1.0f);

    // printf("nk_pugl_render %dx%d\n", pugl->width, pugl->height);
    nk_pugl_render_begin (pugl->width, pugl->height);
    
    {
        GLsizei vs = sizeof(struct nk_pugl_vertex);
        size_t vp = offsetof(struct nk_pugl_vertex, position);
        size_t vt = offsetof(struct nk_pugl_vertex, uv);
        size_t vc = offsetof(struct nk_pugl_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_pugl_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_pugl_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_pugl_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(pugl_vertex_t);
        config.vertex_alignment = NK_ALIGNOF(pugl_vertex_t);
        config.null = dev->null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        // clear command/vertex buffers of last stable view
		nk_buffer_clear(&dev->cmds);
		nk_buffer_clear(&dev->vbuf);
		nk_buffer_clear(&dev->ebuf);
        nk_convert(&pugl->ctx, &dev->cmds, &dev->vbuf, &dev->ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&dev->vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*) nk_buffer_memory_const (&dev->ebuf);
        nk_draw_foreach(cmd, &pugl->ctx, &dev->cmds)
        {
            if (! cmd->elem_count) 
                continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x),
                (GLint)((pugl->height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h))),
                (GLint)(cmd->clip_rect.w),
                (GLint)(cmd->clip_rect.h));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&pugl->ctx);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

NK_INTERN void
nk_pugl_event_handler (PuglView* view, const PuglEvent* event)
{
    nk_pugl* self = (nk_pugl*) puglGetHandle (view);
    nk_pugl* pugl = self;
    struct nk_context* ctx = &pugl->ctx;

    switch(event->type) {
        case PUGL_NOTHING: break;       /**< No event */

        case PUGL_BUTTON_PRESS:
        case PUGL_BUTTON_RELEASE:
        {
            /* optional grabbing behavior */
            if (ctx->input.mouse.grab) {
                ctx->input.mouse.grab = 0;
            } else if (ctx->input.mouse.ungrab) {
                ctx->input.mouse.ungrab = 0;
            }

            /* Button handler */
            const PuglEventButton* ev = (const PuglEventButton*)event;
            int down = (event->type == PUGL_BUTTON_PRESS);
            const int x = ev->x, y = ev->y;
            if (ev->button == 1) {
                if (down) { /* Double-Click Button handler */
                    long dt = nk_timestamp() - pugl->last_button_click;
                    if (dt > NK_PUGL_DOUBLE_CLICK_LO && dt < NK_PUGL_DOUBLE_CLICK_HI)
                        nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, nk_true);
                    pugl->last_button_click = nk_timestamp();
                } else nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, nk_false);
                nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
            } else if (ev->button == 2)
                nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
            else if (ev->button == 3)
                nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
            else if (ev->button == 4)
                nk_input_scroll(ctx, nk_vec2(0,1.0f));
            else if (ev->button == 5)
                nk_input_scroll(ctx, nk_vec2(0,-1.0f));

            puglPostRedisplay (self->view);
        } break;

        case PUGL_CONFIGURE: {
            const PuglEventConfigure* ev = (const PuglEventConfigure*)event;
            pugl->width = (int) ev->width;
            pugl->height = (int) ev->height;
            // printf ("x=%d y=%d w=%d h=%d\n", (int)ev->x, (int)ev->y, (int)ev->width, (int)ev->height);
        } break;

        case PUGL_EXPOSE: {
            #define NK_PUGL_MAX_VERTEX_BUFFER 512 * 1024
            #define NK_PUGL_MAX_ELEMENT_BUFFER 128 * 1024
            nk_input_end (&pugl->ctx);

            if (pugl->expose) {
                pugl->expose (pugl->handle, &pugl->ctx);
            }

            nk_pugl_render (pugl->view, NK_ANTI_ALIASING_ON, 
                            NK_PUGL_MAX_VERTEX_BUFFER, 
                            NK_PUGL_MAX_ELEMENT_BUFFER);
            
            nk_input_begin (&pugl->ctx);
        } break;

        case PUGL_CLOSE: {
            /**< Close view */
            if (pugl->close)
                pugl->close (pugl->handle);
        } break;

        case PUGL_KEY_PRESS:            /**< Key press */
        case PUGL_KEY_RELEASE:          /**< Key release */
        case PUGL_TEXT:                 /**< Character entry */
        case PUGL_ENTER_NOTIFY:         /**< Pointer entered view */
        case PUGL_LEAVE_NOTIFY:         /**< Pointer left view */
            puglPostRedisplay (self->view);
            break;
        
        case PUGL_MOTION_NOTIFY: {
            /* Mouse motion handler */
            PuglEventMotion* ev = (PuglEventMotion*) event;
            const int x = ev->x, y = ev->y;
            // printf("%dx%d\n", x, y);
            nk_input_motion (ctx, x, y);
            if (ctx->input.mouse.grabbed) {
                ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
                ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
            }

            puglPostRedisplay (self->view);
        } break;

        case PUGL_SCROLL:               /**< Scroll */
        case PUGL_FOCUS_IN:             /**< Keyboard focus entered view */
        case PUGL_FOCUS_OUT:            /**< Keyboard focus left view */
            puglPostRedisplay (self->view);
            break;
    }
}

//=============================================================================

NK_API void
nk_pugl_init (nk_pugl* self)
{
    nk_init_default (&self->ctx, 0);

    pugl_device_t* dev = &self->dev;
    nk_buffer_init_default (&dev->cmds);
    nk_buffer_init_default (&dev->vbuf);
    nk_buffer_init_default (&dev->ebuf);

    self->view = puglInit (NULL, NULL);
    puglInitBackend (self->view, puglGlBackend());
    puglInitWindowClass (self->view, "NuklearTest");
    puglInitWindowSize (self->view, self->width, self->height);
    puglInitWindowMinSize (self->view, 10, 10);

    puglInitWindowHint (self->view, PUGL_CONTEXT_VERSION_MAJOR, 2);
    puglInitWindowHint (self->view, PUGL_CONTEXT_VERSION_MINOR, 2);
    puglInitWindowHint (self->view, PUGL_RED_BITS, 8);
    puglInitWindowHint (self->view, PUGL_GREEN_BITS, 8);
    puglInitWindowHint (self->view, PUGL_BLUE_BITS, 8);
    puglInitWindowHint (self->view, PUGL_ALPHA_BITS, 8);
    puglInitWindowHint (self->view, PUGL_DEPTH_BITS, 32);
    puglInitWindowHint (self->view, PUGL_STENCIL_BITS, 8);
    puglInitWindowHint (self->view, PUGL_SAMPLES, 8);
    puglInitWindowHint (self->view, PUGL_DOUBLE_BUFFER, PUGL_TRUE);
    puglInitWindowHint (self->view, PUGL_IGNORE_KEY_REPEAT, PUGL_FALSE);
    puglInitWindowHint (self->view, PUGL_RESIZABLE, PUGL_TRUE);

    puglSetEventFunc (self->view, nk_pugl_event_handler);
    puglSetHandle (self->view, self);
}

NK_API void
nk_pugl_destroy (nk_pugl* self)
{
    puglDestroy (self->view);
    pugl_device_t* dev = &self->dev;
    nk_buffer_free (&dev->cmds);
    nk_buffer_free (&dev->ebuf);
    nk_buffer_free (&dev->vbuf);
    nk_free (&self->ctx);
}

NK_API void
nk_pugl_wait_for_event (nk_pugl* self)
{
    puglWaitForEvent (self->view);
}

NK_API void
nk_pugl_process_events (nk_pugl* self)
{
    nk_input_begin (&self->ctx);
    puglProcessEvents (self->view);
    nk_input_end (&self->ctx);
}

NK_API void
nk_pugl_font_stash_begin(nk_pugl *pugl)
{
    nk_font_atlas_init_default(&pugl->atlas);
    nk_font_atlas_begin(&pugl->atlas);
}

NK_API void
nk_pugl_font_stash_end(nk_pugl *pugl)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&pugl->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_pugl_device_upload_atlas(pugl, image, w, h);
    nk_font_atlas_end(&pugl->atlas, nk_handle_id((int)pugl->dev.font_tex), &pugl->dev.null);
    if (pugl->atlas.default_font)
        nk_style_set_font(&pugl->ctx, &pugl->atlas.default_font->handle);
}

#ifdef __cpluspus
  }
#endif
#endif
