/** FPGA card
 *
 * This class represents a FPGA device.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Daniel Krebs <github@daniel-krebs.net>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

/** @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

#include <list>
#include <set>
#include <string>
#include <jansson.h>

#include <villas/plugin.hpp>
#include <villas/memory.hpp>

#include <villas/kernel/pci.h>
#include <villas/kernel/vfio.hpp>

#include <villas/fpga/config.h>
#include <villas/fpga/core.hpp>

#define PCI_FILTER_DEFAULT_FPGA {		\
	.id = {								\
	    .vendor = FPGA_PCI_VID_XILINX,	\
	    .device = FPGA_PCI_PID_VFPGA,	\
	    .class_code = 0					\
	},									\
	.slot = { }							\
}

namespace villas {
namespace fpga {


/* Forward declarations */
struct vfio_container;
class PCIeCardFactory;

class PCIeCard {
public:

	friend PCIeCardFactory;

	PCIeCard() : filter(PCI_FILTER_DEFAULT_FPGA) {}
	~PCIeCard();

	bool init();
	bool stop()  { return true; }
	bool check() { return true; }
	bool reset() { return true; }
	void dump()  { }

	ip::Core::Ptr lookupIp(const std::string& name) const;
	ip::Core::Ptr lookupIp(const Vlnv& vlnv) const;
	ip::Core::Ptr lookupIp(const ip::IpIdentifier& id) const;

	bool
	mapMemoryBlock(const MemoryBlock& block);

private:
	/// Cache a set of already mapped memory blocks
	std::set<MemoryManager::AddressSpaceId> memoryBlocksMapped;

public:	// TODO: make this private
	ip::Core::List ips;		///< IPs located on this FPGA card

	bool do_reset;			/**< Reset VILLASfpga during startup? */
	int affinity;			/**< Affinity for MSI interrupts */

	std::string name;			/**< The name of the FPGA card */

	struct pci* pci;
	struct pci_device filter;		/**< Filter for PCI device. */
	struct pci_device* pdev;		/**< PCI device handle */

	/// The VFIO container that this card is part of
	std::shared_ptr<VfioContainer> vfioContainer;

	/// The VFIO device that represents this card
	VfioDevice* vfioDevice;

	/// Slave address space ID to access the PCIe address space from the FPGA
	MemoryManager::AddressSpaceId addrSpaceIdDeviceToHost;

	/// Address space identifier of the master address space of this FPGA card.
	/// This will be used for address resolution of all IPs on this card.
	MemoryManager::AddressSpaceId addrSpaceIdHostToDevice;

protected:
	Logger
	getLogger() const
	{ return villas::logging.get(name); }

	Logger logger;
};

using CardList = std::list<std::shared_ptr<PCIeCard>>;

class PCIeCardFactory : public plugin::Plugin {
public:

	static CardList
	make(json_t *json, struct pci* pci, std::shared_ptr<VfioContainer> vc);

	static PCIeCard*
	create();

	static Logger
	getStaticLogger()
	{ return villas::logging.get("PCIeCardFactory"); }

	virtual std::string
	getName() const
	{ return "pcie"; }

	virtual std::string
	getDescription() const
	{ return "Xilinx PCIe FPGA cards"; } 
};

} /* namespace fpga */
} /* namespace villas */

/** @} */
