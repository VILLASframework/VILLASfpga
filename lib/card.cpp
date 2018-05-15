/** FPGA card.
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

#include <string>
#include <memory>
#include <utility>

#include "log.hpp"

#include "kernel/pci.h"
#include "kernel/vfio.hpp"

#include "fpga/ip.hpp"
#include "fpga/card.hpp"

namespace villas {
namespace fpga {

// instantiate factory to register
static PCIeCardFactory PCIeCardFactory;

CardList
PCIeCardFactory::make(json_t *json, struct pci* pci, std::shared_ptr<VfioContainer> vc)
{
	CardList cards;
	auto logger = getStaticLogger();

	const char *card_name;
	json_t *json_card;
	json_object_foreach(json, card_name, json_card) {
		logger->info("Found config for FPGA card {}", card_name);

		json_t*     json_ips = nullptr;
		const char* pci_slot = nullptr;
		const char* pci_id   = nullptr;
		int         do_reset = 0;
		int         affinity = 0;

		int ret = json_unpack(json_card, "{ s: o, s?: i, s?: b, s?: s, s?: s }",
		    "ips", &json_ips,
		    "affinity", &affinity,
		    "do_reset", &do_reset,
		    "slot", &pci_slot,
		    "id", &pci_id);

		if(ret != 0) {
			logger->warn("Cannot parse JSON config");
			continue;
		}

		auto card = std::unique_ptr<PCIeCard>(create());

		// populate generic properties
		card->name = std::string(card_name);
		card->pci = pci;
		card->vfioContainer = std::move(vc);
		card->affinity = affinity;
		card->do_reset = do_reset != 0;

		const char* error;

		if (pci_slot != nullptr and pci_device_parse_slot(&card->filter, pci_slot, &error) != 0) {
			logger->warn("Failed to parse PCI slot: {}", error);
		}

		if (pci_id != nullptr and pci_device_parse_id(&card->filter, pci_id, &error) != 0) {
			logger->warn("Failed to parse PCI ID: {}", error);
		}


		if(not card->init()) {
			logger->warn("Cannot start FPGA card {}", card_name);
			continue;
		}

		card->ips = ip::IpCoreFactory::make(card.get(), json_ips);
		if(card->ips.empty()) {
			logger->error("Cannot initialize IPs of FPGA card {}", card_name);
			continue;
		}

		if(not card->check()) {
			logger->warn("Checking of FPGA card {} failed", card_name);
			continue;
		}

		cards.push_back(std::move(card));
	}

	return cards;
}


PCIeCard*
PCIeCardFactory::create()
{
	return new fpga::PCIeCard;
}


PCIeCard::~PCIeCard()
{
	auto& mm = MemoryManager::get();

	// unmap all memory blocks
	for(auto& mappedMemoryBlock : memoryBlocksMapped) {
		auto translation = mm.getTranslation(addrSpaceIdDeviceToHost,
		                                     mappedMemoryBlock);

		const uintptr_t iova = translation.getLocalAddr(0);
		const size_t size = translation.getSize();

		logger->debug("Unmap block {} at IOVA {:#x} of size {:#x}",
		              mappedMemoryBlock, iova, size);
		vfioContainer->memoryUnmap(iova, size);
	}
}


ip::IpCore*
PCIeCard::lookupIp(const std::string& name) const
{
	for(auto& ip : ips) {
		if(*ip == name) {
			return ip.get();
		}
	}

	return nullptr;
}


ip::IpCore*
PCIeCard::lookupIp(const Vlnv& vlnv) const
{
	for(auto& ip : ips) {
		if(*ip == vlnv) {
			return ip.get();
		}
	}

	return nullptr;
}


bool
PCIeCard::mapMemoryBlock(const MemoryBlock& block)
{
	auto& mm = MemoryManager::get();
	const auto& addrSpaceId = block.getAddrSpaceId();

	if(memoryBlocksMapped.find(addrSpaceId) != memoryBlocksMapped.end()) {
		// block already mapped
		return true;
	} else {
		logger->debug("Create VFIO mapping for {}", addrSpaceId);
	}


	auto translationFromProcess = mm.getTranslationFromProcess(addrSpaceId);
	uintptr_t processBaseAddr = translationFromProcess.getLocalAddr(0);
	uintptr_t iovaAddr = vfioContainer->memoryMap(processBaseAddr,
	                                              UINTPTR_MAX,
	                                              block.getSize());

	if(iovaAddr == UINTPTR_MAX) {
		logger->error("Cannot map memory at {:#x} of size {:#x}",
		              processBaseAddr, block.getSize());
		return false;
	}



	mm.createMapping(iovaAddr, 0, block.getSize(),
	                 "vfio",
	                 this->addrSpaceIdDeviceToHost,
	                 addrSpaceId);

	// remember that this block has already been mapped for later
	memoryBlocksMapped.insert(addrSpaceId);

	return true;
}


bool
PCIeCard::init()
{
	auto& mm = MemoryManager::get();
	logger = getLogger();

	logger->info("Initializing FPGA card {}", name);

	/* Search for FPGA card */
	pdev = pci_lookup_device(pci, &filter);
	if (!pdev) {
		logger->error("Failed to find PCI device");
		return false;
	}

	/* Attach PCIe card to VFIO container */
	VfioDevice& device = vfioContainer->attachDevice(pdev);
	this->vfioDevice = &device;


	/* Enable memory access and PCI bus mastering for DMA */
	if (not device.pciEnable()) {
		logger->error("Failed to enable PCI device");
		return false;
	}

	/* Map PCIe BAR */
	const void* bar0_mapped = vfioDevice->regionMap(VFIO_PCI_BAR0_REGION_INDEX);
	if (bar0_mapped == MAP_FAILED) {
		logger->error("Failed to mmap() BAR0");
		return false;
	}

	// determine size of BAR0 region
	const size_t bar0_size = vfioDevice->regionGetSize(VFIO_PCI_BAR0_REGION_INDEX);


	/* Link mapped BAR0 to global memory graph */

	// get the address space of the current application
	const auto villasAddrSpace = mm.getProcessAddressSpace();

	// get the address space for the PCIe proxy we use with VFIO
	const auto cardPCIeAddrSpaceName = mm.getMasterAddrSpaceName(name, "PCIe");

	// create a new address space for this FPGA card
	addrSpaceIdHostToDevice = mm.getOrCreateAddressSpace(cardPCIeAddrSpaceName);

	// create a mapping from our address space to the FPGA card via vfio
	mm.createMapping(reinterpret_cast<uintptr_t>(bar0_mapped),
	                                   0, bar0_size, "VFIO_map",
	                                   villasAddrSpace, addrSpaceIdHostToDevice);


	/* Reset system? */
	if (do_reset) {
		/* Reset / detect PCI device */
		if(not vfioDevice->pciHotReset()) {
			logger->error("Failed to reset PCI device");
			return false;
		}

		if(not reset()) {
			logger->error("Failed to reset FGPA card");
			return false;
		}
	}

	return true;
}

} // namespace fpga
} // namespace villas
