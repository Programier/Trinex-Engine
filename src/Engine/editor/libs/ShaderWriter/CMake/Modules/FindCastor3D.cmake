# FindCastor3D
# ------------
#
# Locate Castor3D library
#
# This module defines
#
# ::
#
#   castor3d_copy_files, function to copy Castor3D's shared libraries to client target's output folders.
#

function( _copy_files _TARGET_NAME _TARGET_DIR_RELEASE _TARGET_DIR_RELWITHDEBINFO _TARGET_DIR_DEBUG _SOURCE_RELEASE _SOURCE_DEBUG _DESTINATION )
	get_filename_component( _FILE ${_SOURCE_RELEASE} NAME_WE )
	get_filename_component( _LIB_NAME_RELEASE ${_SOURCE_RELEASE} NAME )
	get_filename_component( _LIB_NAME_DEBUG ${_SOURCE_DEBUG} NAME )
	add_custom_command(
		TARGET ${_TARGET_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${_SOURCE_DEBUG}>
			$<$<CONFIG:Debug>:${_TARGET_DIR_DEBUG}/${_DESTINATION}/${_LIB_NAME_DEBUG}>
			$<$<CONFIG:Release>:${_SOURCE_RELEASE}>
			$<$<CONFIG:Release>:${_TARGET_DIR_RELEASE}/${_DESTINATION}/${_LIB_NAME_RELEASE}>
			$<$<CONFIG:RelWithDebInfo>:${_SOURCE_RELEASE}>
			$<$<CONFIG:RelWithDebInfo>:${_TARGET_DIR_RELWITHDEBINFO}/${_DESTINATION}/${_LIB_NAME_RELEASE}>
	)
	install(
		FILES ${_SOURCE_RELEASE}
		DESTINATION ${_DESTINATION}
		COMPONENT ${_TARGET_NAME}
		CONFIGURATIONS Release RelWithDebInfo
	)
	install(
		FILES ${_SOURCE_DEBUG}
		DESTINATION ${_DESTINATION}
		COMPONENT ${_TARGET_NAME}
		CONFIGURATIONS Debug
	)
endfunction()

function( _copy_target_files _TARGET _DESTINATION )# ARGN: The files
	if ( NOT "${_DESTINATION}" STREQUAL "" )
		set( _DESTINATION ${_DESTINATION}/ )
	endif ()
	foreach ( _FILE ${ARGN} )
		get_filename_component( _FILE ${_FILE} REALPATH )
		get_filename_component( _FILE_NAME ${_FILE} NAME )
		add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/Castor3D/${_DESTINATION}
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_FILE} ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/Castor3D/${_DESTINATION}${_FILE_NAME}
			COMMENT "Copying ${_FILE} into ${_DESTINATION} folder"
		)
	endforeach ()
	install(
		FILES ${ARGN}
		DESTINATION share/${_TARGET}
		COMPONENT ${_TARGET}
	)
endfunction()

function( castor3d_copy_files _TARGET_NAME _TARGET_DIR_RELEASE _TARGET_DIR_RELWITHDEBINFO _TARGET_DIR_DEBUG )
	if ( WIN32 )
		include( InstallRequiredSystemLibraries )
		set( castorTargets
			castor::CastorUtils
			castor::Castor3D
			castor::SceneExporter
			ashes::ashes
			ashes::ashesCommon
			ashes::ashespp
			ashes::ashesD3D11Renderer
			ashes::ashesGlRenderer
			ashes::ashesVkRenderer
			ashes::ashesTestRenderer
			sdw::ShaderAST
			sdw::ShaderWriter
			sdw::CompilerGlsl
			sdw::CompilerSpirV
			sdw::CompilerHlsl
			crg::RenderGraph
		)
		foreach( castorTarget ${castorTargets} )
			if ( TARGET ${castorTarget} )
				get_target_property( TARGET_BIN_RELEASE ${castorTarget} IMPORTED_LOCATION_RELEASE )
				get_target_property( TARGET_BIN_DEBUG ${castorTarget} IMPORTED_LOCATION_DEBUG )
				get_target_property( TARGET_LIB_RELEASE ${castorTarget} IMPORTED_IMPLIB_RELEASE )
				get_target_property( TARGET_LIB_DEBUG ${castorTarget} IMPORTED_IMPLIB_DEBUG )
				if ( EXISTS ${TARGET_LIB_RELEASE} )
					_copy_files( ${_TARGET_NAME}
						${_TARGET_DIR_RELEASE}
						${_TARGET_DIR_RELWITHDEBINFO}
						${_TARGET_DIR_DEBUG}
						${TARGET_BIN_RELEASE}
						${TARGET_BIN_DEBUG}
						bin
					)
					_copy_files( ${_TARGET_NAME}
						${_TARGET_DIR_RELEASE}
						${_TARGET_DIR_RELWITHDEBINFO}
						${_TARGET_DIR_DEBUG}
						${TARGET_LIB_RELEASE}
						${TARGET_LIB_DEBUG}
						lib
					)
				else ()
					_copy_files( ${_TARGET_NAME}
						${_TARGET_DIR_RELEASE}
						${_TARGET_DIR_RELWITHDEBINFO}
						${_TARGET_DIR_DEBUG}
						${TARGET_BIN_RELEASE}
						${TARGET_BIN_DEBUG}
						lib
					)
				endif ()
			endif ()
		endforeach ()
		set( castorPlugins
			castor::AssimpImporter
			castor::PlyImporter
			castor::PnTrianglesDivider
			castor::LoopDivider
			castor::PhongDivider
			castor::LinearToneMapping
			castor::HaarmPieterDuikerToneMapping
			castor::HejlBurgessDawsonToneMapping
			castor::ReinhardToneMapping
			castor::Uncharted2ToneMapping
			castor::ACESToneMapping
			castor::BloomPostEffect
			castor::GrayScalePostEffect
			castor::FxaaPostEffect
			castor::SmaaPostEffect
			castor::FilmGrainPostEffect
			castor::LightStreaksPostEffect
			castor::LinearMotionBlurPostEffect
			castor::DrawEdgesPostEffect
			castor::CastorGui
			castor::ToonMaterial
			castor::WaterRendering
			castor::OceanRendering
			castor::FFTOceanRendering
			castor::FireworksParticle
			castor::DiamondSquareTerrain
		)
		foreach( castorPlugin ${castorPlugins} )
			if ( TARGET ${castorPlugin} )
				get_target_property( TARGET_BIN_RELEASE ${castorPlugin} IMPORTED_LOCATION_RELEASE )
				get_target_property( TARGET_BIN_DEBUG ${castorPlugin} IMPORTED_LOCATION_DEBUG )
				_copy_files( ${_TARGET_NAME}
					${_TARGET_DIR_RELEASE}
					${_TARGET_DIR_RELWITHDEBINFO}
					${_TARGET_DIR_DEBUG}
					${TARGET_BIN_RELEASE}
					${TARGET_BIN_DEBUG}
					bin/Castor3D
				)
				get_target_property( TARGET_LIB_RELEASE ${castorPlugin} IMPORTED_IMPLIB_RELEASE )
				get_target_property( TARGET_LIB_DEBUG ${castorPlugin} IMPORTED_IMPLIB_DEBUG )
				_copy_files( ${_TARGET_NAME}
					${_TARGET_DIR_RELEASE}
					${_TARGET_DIR_RELWITHDEBINFO}
					${_TARGET_DIR_DEBUG}
					${TARGET_LIB_RELEASE}
					${TARGET_LIB_DEBUG}
					lib/Castor3D
				)
			endif ()
		endforeach ()
	endif ()
	if ( TARGET castor::Castor3D )
		get_target_property( TARGET_BIN_RELEASE castor::Castor3D IMPORTED_LOCATION_RELEASE )
		get_filename_component( TARGET_BIN_RELEASE ${TARGET_BIN_RELEASE} DIRECTORY )
		get_filename_component( Castor3D_ROOT_DIR ${TARGET_BIN_RELEASE} DIRECTORY )
		set( Castor3D_SHARE_DIR "${Castor3D_ROOT_DIR}/share/Castor3D" )
		file(
			GLOB
				CoreZipFiles
				${Castor3D_SHARE_DIR}/*.zip
		)
		_copy_target_files( ${_TARGET_NAME} "" ${CoreZipFiles} )
	endif ()
endfunction()
