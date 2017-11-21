/** Loadable / plugin support.
 *
 * @file
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

#pragma once

#include "utils.h"
#include "fpga/ip.h"

/** @todo This is ugly as hell and broken on OS X / Clang anyway. */
#define REGISTER_PLUGIN(p)					\
__attribute__((constructor(110))) static void UNIQUE(__ctor)() {\
	if (plugins.state == STATE_DESTROYED)			\
		list_init(&plugins);				\
	list_push(&plugins, p);					\
}								\
__attribute__((destructor(110))) static void UNIQUE(__dtor)() {	\
	if (plugins.state != STATE_DESTROYED)			\
		list_remove(&plugins, p);			\
}

extern struct list plugins;

enum plugin_type {
	PLUGIN_TYPE_FPGA_IP,
};

struct plugin {
	const char *name;
	const char *description;
	void *handle;
	char *path;

	enum plugin_type type;

	enum state state;

	int (*load)(struct plugin *p);
	int (*unload)(struct plugin *p);

	struct fpga_ip_type	ip;
};

/** Return a pointer to the plugin structure */
#define plugin(vt) ((struct plugin *) ((char *) (vt) - offsetof(struct plugin, api)))

#define plugin_name(vt) plugin(vt)->name
#define plugin_description(vt) plugin(vt)->description

int plugin_init(struct plugin *p);

int plugin_destroy(struct plugin *p);

int plugin_parse(struct plugin *p, json_t *cfg);

int plugin_load(struct plugin *p);

int plugin_unload(struct plugin *p);

void plugin_dump(enum plugin_type type);

/** Find registered and loaded plugin with given name and type. */
struct plugin * plugin_lookup(enum plugin_type type, const char *name);
