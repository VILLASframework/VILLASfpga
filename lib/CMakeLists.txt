## CMakeLists.txt
#
# @author Daniel Krebs <github@daniel-krebs.net>
# @copyright 2018, RWTH Institute for Automation of Complex Power Systems (ACS)
# @license GNU General Public License (version 3)
#
# VILLASfpga
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

set(SOURCES
	vlnv.cpp
	card.cpp
	core.cpp
	node.cpp

	ips/timer.cpp
	ips/switch.cpp
	ips/emc.cpp
	ips/fifo.cpp
	ips/intc.cpp
	ips/pcie.cpp
	ips/dma.cpp
	ips/bram.cpp
	ips/rtds.cpp
	ips/aurora.cpp
	ips/gpio.cpp

	ips/rtds2gpu/rtds2gpu.cpp
	ips/rtds2gpu/xrtds2gpu.c
	ips/rtds2gpu/gpu2rtds.cpp
)

# we don't have much influence on drivers generated by Xilinx, so ignore warnings
set_source_files_properties(ips/rtds2gpu/xrtds2gpu.c
	PROPERTIES COMPILE_FLAGS -Wno-int-to-pointer-cast)

add_library(villas-fpga SHARED ${SOURCES})

target_link_libraries(villas-fpga PUBLIC villas-common)

target_compile_definitions(villas-fpga PRIVATE
	BUILDID=\"abc\"
	_GNU_SOURCE
)

target_include_directories(villas-fpga
	PUBLIC
		${PROJECT_BINARY_DIR}/include
		${PROJECT_SOURCE_DIR}/include
		${XIL_INCLUDE_DIRS}
		${JANSSON_INCLUDE_DIRS}
)

target_link_libraries(villas-fpga PUBLIC
	${XIL_LINK_LIBRARIES}
	${JANSSON_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
	m
	villas-common
)

if(CMAKE_CUDA_COMPILER)
	target_link_libraries(villas-fpga PUBLIC villas-gpu)
endif()

include(GNUInstallDirs)

install(TARGETS villas-fpga
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)

install(DIRECTORY ../include/villas DESTINATION include)


