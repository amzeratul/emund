cmake_minimum_required (VERSION 3.10)

project (emund)

set(HALLEY_PROJECT_EMBED 1)
set(HALLEY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/halley)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${HALLEY_PATH}/cmake/")
include(HalleyProject)

if(NOT (DEFINED HALLEY_GAME_BIN_DIR))
	set(HALLEY_GAME_BIN_DIR "bin")
endif()

set (SOURCES
	"prec.cpp"
	
	"src/cpu/address_space.cpp"
	"src/cpu/cpu_6502.cpp"
	"src/cpu/cpu_6502_disassembler.cpp"
	
	"src/game/emund_game.cpp"
	"src/game/game_stage.cpp"
	
	"src/nes/nes_mapper.cpp"
	"src/nes/nes_machine.cpp"
	"src/nes/nes_rom.cpp"
	)

set (HEADERS
	"prec.h"
	
	"src/cpu/address_space.h"
	"src/cpu/cpu_6502.h"
	"src/cpu/cpu_6502_disassembler.h"
	
	"src/game/emund_game.h"
	"src/game/game_stage.h"

	"src/nes/nes_mapper.h"
	"src/nes/nes_machine.h"
	"src/nes/nes_rom.h"

	"src/utils/macros.h"
	)

set (GEN_DEFINITIONS
	)

halleyProject(emund "${SOURCES}" "${HEADERS}" "${GEN_DEFINITIONS}" ${CMAKE_CURRENT_SOURCE_DIR}/${HALLEY_GAME_BIN_DIR})
