/**
ROS/IOP Bridge
Copyright (c) 2017 Fraunhofer

This program is dual licensed; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation, or
enter into a proprietary license agreement with the copyright
holder.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; or you can read the full license at
<http://www.gnu.de/documents/gpl-2.0.html>
*/

/** \author Alexander Tiderko */

#ifndef IOP_PLUGIN_INTERFACE_H_
#define IOP_PLUGIN_INTERFACE_H_

#include <typeinfo>
#include "Service.h"
#include "Transport/JausTransport.h"

namespace iop
{
	class PluginInterface
	{
		/**
		 * The interface is used by the iop_component_fkie to create the services and include
		 * these to a component. Each IOP service should include this interface to create a
		 * plugin.
		 * The URL, ID and version of the service are specified in plugin_iop.xml with follow
		 * format:
		 * <library path="libiop_events_1_0_fkie">
		 * 	<class name="Events_1_0" type="iop::EventsPlugin_1_0" base_class_type="iop::PluginInterface">
		 * 		<description> Events Service Plugin </description>
		 * 		<iop_service name="Events" id="urn:jaus:jss:core:Events" version="1.0">
		 * 			<inherits_from id="urn:jaus:jss:core:Transport" min_version="1.0"/>
		 * 		</iop_service>
		 * 	</class>
		 * </library>
		 *
		 * The plugins are loaded by iop_component. Thereby the defined base plugins are set
		 * before call create_service(...).
		 */
	public:
		/**
		 * ServiceInfo is filled by iop_component with values from plugin_iop.xml.
		 */
		struct ServiceInfo {
			std::string name;  // <iop_service name<=
			std::string id;  //  <iop_service id<=
			unsigned char version_manjor;  //  <iop_service version<=
			unsigned char version_minor;    //  <iop_service version<=
			std::string inherits_from;  //  <inherits_from id<=
			unsigned char inherits_from_version_manjor;  //  <inherits_from version<=
			unsigned char inherits_from_min_version_minor;  //  <inherits_from version<=
			std::vector<std::string> depend; // <depend id<=
		};

		// ============================
		// === must be overridden =====
		// ____________________________
		/** This method is called to create the JTS service managed by this plugin.
		 * Previously, the base plugin is set by the caller.
		 * JTS: a service generated by JAUS Tool Set */
		virtual void create_service(JTS::JausRouter* jausRouter) = 0;
		/** This method should return a right JTS service object after create_service()
		 * was called. The returned object will be deleted by caller! In our case iop_comopnent.*/
		virtual JTS::Service* get_service() = 0;
		// ____________________________
		// === must be overridden =====
		// ============================


		// ============================
		// === can be overridden =====
		// ____________________________
		/** The init_service is called after all plugins are loaded and
		 * create_service() of each plugin was called. You can call
		 * Component::get_service(std::string service_name) to find a service if it was loaded. */
		virtual void init_service() {}
		/** This methods must be implemented only by discovery client plugins. */
		virtual bool is_discovery_client() { return false; }
		/** Register the JTS service by discovery service in the IOP/JAUS system. */
		virtual void register_service(PluginInterface *plugin) {}
		virtual ~PluginInterface(){}
		// ____________________________
		// === can be overridden =====
		// ============================

		/** Sets the service info. It should be done by the iop_component before the
		 * create_service() is called. */
		void set_service_info(ServiceInfo service_info) { p_service_info = service_info; }

		/** Returns the name of the JTS service managed by this plugin. */
		const std::string get_service_name() { return p_service_info.name; }
		/** Service URL defined by JAUS ID */
		jVariableLengthString get_service_uri() { return p_service_info.id; }
		/** Version of the IOP service managed by this plugin. */
		jUnsignedByte get_version_number_major() { return p_service_info.version_manjor; }
		jUnsignedByte get_version_number_minor() { return p_service_info.version_minor; }

		/** The caller object sets the base plugin before call the create_jts_service(). */
		void set_base_plugin(PluginInterface *base_plugin) { p_base_plugin = base_plugin; }
		/** Returns the id of the based JTS service or an empty string if not defined. */
		const std::string get_base_service_uri() { return p_service_info.inherits_from; }
		std::vector<std::string> get_depends() { return p_service_info.depend; }
		unsigned char get_base_version_manjor() { return p_service_info.inherits_from_version_manjor; }
		unsigned char get_base_min_version_minor() { return p_service_info.inherits_from_min_version_minor; }
		/** Returns the base plugin or NULL if not defined. */
		PluginInterface *get_base_plugin() { return p_base_plugin; }
		/** Returns the JTS service on which the managed service is based.
		 * param depth: the depth of the based service
		 * 	0 - NULL
		 * 	1 - based service
		 * 	2 - based service of the based service*/
		JTS::Service* get_base_service(int depth=1)
		{
			int index = 0;
			PluginInterface *base_plugin = get_base_plugin();
			JTS::Service* result = NULL;
			while (index < depth)
			{
				if (base_plugin != NULL) {
					result = base_plugin->get_service();
					base_plugin = base_plugin->get_base_plugin();
				}
				index++;
			}
			return result;
		}
		// this variable is used from outside while initialization
		std::string error_message;
	protected:
		PluginInterface() { p_base_plugin = NULL; }
		PluginInterface *p_base_plugin;
		ServiceInfo p_service_info;
	};
};

#endif
