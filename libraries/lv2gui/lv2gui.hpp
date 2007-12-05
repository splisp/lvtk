/****************************************************************************
    
    lv2gui.hpp - Wrapper library to make it easier to write LV2 GUIs in C++
    
    Copyright (C) 2006-2007 Lars Luthman <lars.luthman@gmail.com>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301  USA

****************************************************************************/

#ifndef LV2GUI_HPP
#define LV2GUI_HPP

#include <map>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/widget.h>

#include "lv2-gui.h"


namespace LV2 {

  /** This class is an interface for controlling a plugin instance. An object
      of this class will be provided by the host when a plugin GUI is 
      instantiated. */
  class Controller {
  public:
    
    /** This constructor creates an invalid controller that doesn't actually
	control anything. */
    Controller();
    
    /** Write to a port in the plugin instance. */
    void write(uint32_t port, uint32_t buffer_size, const void* buffer);
    
    /** Convenient wrapper for writing to control ports. */
    inline void write_control(uint32_t port, const float& value) {
      write(port, sizeof(float), &value);
    }
    
    /** Send a command to the plugin instance. */
    void command(uint32_t argc, const char* const* argv);
    
    /** Tell the plugin host to switch to program @c number for the plugin
	instance. */
    void request_program(unsigned char number);
    
    /** Tell the plugin host to save the current plugin state to a program. */
    void request_save(unsigned char number, const char* name);
    
    /** Return data associated with an extension URI, or 0 if that extension
	is not supported or does not have any data for use in controllers. */
    void* extension_data(const std::string& URI);
    
  protected:
    
    friend class GUI;
    
    Controller(LV2UI_Write_Function wfcn, LV2UI_Command_Function cfcn,
	       LV2UI_Program_Change_Function pfcn, 
	       LV2UI_Program_Save_Function sfcn, LV2UI_Controller ctrl, 
	       const LV2_Feature* const* features);
    
    LV2UI_Write_Function m_wfunc;
    LV2UI_Command_Function m_cfunc;
    LV2UI_Program_Change_Function m_pfunc;
    LV2UI_Program_Save_Function m_sfunc;
    LV2UI_Controller m_ctrl;
  };
  

  /** This is the base class for a plugin GUI. You should inherit it and 
      override any functions you want to provide implementations for. 
      All subclasses must have a constructor with the signature
      
      MyGUI(LV2::Controller& controller, const char* plugin_URI, 
            const char* bundle_path, Gtk::Widget*& UI_widget);
	    
      Where @c controller is the interface for controlling the plugin instance
      and @c UI_widget should be set to point to a widget that the host will
      display to the user. The plugin is responsible for deallocating the 
      widget when the destructor for the LV2GUI subclass is called. */
  class GUI : public Gtk::HBox {
  public:
    
    virtual ~GUI() { delete m_controller; }
    
    /** Override this if you want your GUI to do something when a control port
	value changes in the plugin instance. */
    virtual void port_event(uint32_t port, uint32_t buffer_size, 
			    const void* buffer) { }
    
    virtual void feedback(uint32_t argc, const char* const* argv) { }
    
    /** Override this if you want your GUI to do something when a program has
	been added for the plugin instance. */
    virtual void program_added(unsigned char number, const char* name) { }
    
    /** Override this if you want your GUI to do something when a program has
	been removed for the plugin instance. */
    virtual void program_removed(unsigned char number) { }
    
    /** Override this if you want your GUI to do something when the host
	removes all programs for the plugin instance. */
    virtual void programs_cleared() { }
    
    /** Override this if you want your GUI to do something when the host
	changes the program for the plugin instance. */
    virtual void current_program_changed(unsigned char number) { }
    
    /** Return data associated with an extension URI, or 0 if that extension
	is not supported or does not have any data for use in plugin GUIs. */
    virtual void* extension_data(const std::string& URI) { return 0; }

    /** Use this template function to register a class as a LV2 GUI. */
    template <typename T> static int register_class(const std::string& URI) {
      LV2UI_Descriptor* desc = new LV2UI_Descriptor;
      std::memset(desc, 0, sizeof(LV2UI_Descriptor));
      desc->URI = strdup(URI.c_str());
      desc->instantiate = &create_ui_instance<T>;
      desc->cleanup = &delete_ui_instance;
      desc->port_event = &port_event;
      desc->feedback = &feedback;
      desc->program_added = &program_added;
      desc->program_removed = &program_removed;
      desc->programs_cleared = &programs_cleared;
      desc->current_program_changed = &current_program_changed;
      desc->extension_data = &extension_data;
      get_lv2g2g_descriptors().push_back(desc);
      return get_lv2g2g_descriptors().size() - 1;
    }
    
    typedef std::vector<LV2UI_Descriptor*> DescList;
    
    /** @internal
	This function returns the list of LV2 descriptors. It should only be 
	used internally. */
    static DescList& get_lv2g2g_descriptors();

  private:
    
    /** @internal
	This template function creates an instance of a plugin GUI. It is used 
	as the instantiate() callback in the LV2 descriptor. You should not use
	it directly. */
    template <class T>
    static LV2UI_Handle create_ui_instance(const struct _LV2UI_Descriptor* descriptor,
				    const char*                     plugin_uri,
				    const char*                     bundle_path,
				    LV2UI_Write_Function            write_func,
				    LV2UI_Command_Function          command_func,
				    LV2UI_Program_Change_Function   program_func,
				    LV2UI_Program_Save_Function     save_func,
				    LV2UI_Controller                ctrl,
				    LV2UI_Widget*                   widget,
				    const LV2_Feature* const*       features) {
      
      // this is needed to initialise gtkmm stuff in case we're running in
      // a Gtk+ or PyGtk host or some other language
      Gtk::Main::init_gtkmm_internals();
      
      // create a Controller object that wraps all the host callbacks
      Controller* controller = new Controller(write_func, command_func,
					      program_func, save_func,
					      ctrl, features);
      
      // create the GUI object
      T* t = new T(*controller, plugin_uri, bundle_path);
      
      // the GUI object assumes ownership of the Controller pointer and will delete
      // it in its destructor
      t->m_controller = controller;
      
      *widget = static_cast<Gtk::Widget*>(t)->gobj();
      return reinterpret_cast<LV2UI_Handle>(t);
    }
    

    /** @internal
	This function destroys an instance of a GUI. It is used as the
	cleanup() callback in the LV2UI descriptor. You should not use it directly. */
    static void delete_ui_instance(LV2UI_Handle instance);
    
    static void port_event(LV2UI_Handle instance, uint32_t port, uint32_t buffer_size,
			   const void* buffer);
    
    static void feedback(LV2UI_Handle instance, uint32_t argc, 
			 const char* const* argv);
    
    static void program_added(LV2UI_Handle instance, unsigned char number, 
			      const char* name);
    
    static void program_removed(LV2UI_Handle instance, unsigned char number);
    
    static void programs_cleared(LV2UI_Handle instance);
    
    static void current_program_changed(LV2UI_Handle instance, unsigned char number);
    
    static void* extension_data(LV2UI_Handle instance, const char* URI);
    
    
    Controller* m_controller;
    
  };
  
  
}


#endif
