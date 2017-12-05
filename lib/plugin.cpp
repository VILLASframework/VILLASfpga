/** Loadable / plugin support.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASfpga
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <dlfcn.h>
#include <string.h>

#include <string>
#include <iostream>
#include <iostream>

#include "plugin.hpp"


namespace villas {

// list of all registered plugins
Plugin::PluginList Plugin::pluginList;


Plugin::Plugin() :
    name(""),
    description(""),
    path(""),
    pluginType(Plugin::Type::Unknown),
    state(STATE_INITIALIZED)
{
	// push to global plugin list
	pluginList.push_back(this);
}

Plugin::~Plugin()
{
	// clean from global plugin list
	pluginList.remove(this);
}


int
Plugin::parse(json_t *cfg)
{
	const char *path;

	path = json_string_value(cfg);
	if (!path)
		return -1;

	this->path = std::string(path);
	this->state = STATE_PARSED;

	return 0;
}

int
Plugin::load()
{
	assert(this->state == STATE_PARSED);
	assert(not this->path.empty());

	this->handle = dlopen(this->path.c_str(), RTLD_NOW);

	if (this->handle == nullptr)
		return -1;

	this->state = STATE_LOADED;

	return 0;
}

int
Plugin::unload()
{
	int ret;

	assert(this->state == STATE_LOADED);

	ret = dlclose(this->handle);
	if (ret != 0)
		return -1;

	this->state = STATE_UNLOADED;

	return 0;
}

void
Plugin::dump()
{
	std::cout << " - " << this->name << ": " << this->description << std::endl;
}

Plugin*
Plugin::lookup(Plugin::Type type, std::string name)
{
	for(auto& p : pluginList) {
		if(p->pluginType == type and p->name == name)
			return p;
	}

	return nullptr;
}

std::list<Plugin*>
Plugin::lookup(Plugin::Type type)
{
	std::list<Plugin*> list;
	for(auto& p : pluginList) {
		if(p->pluginType == type)
			list.push_back(p);
	}

	return list;
}

bool
Plugin::operator==(const Plugin &other) const
{
	return (this->pluginType == other.pluginType) and (this->name == other.name);
}

} // namespace villas
