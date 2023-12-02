#--------------------------------------------------------------------------
# engine
#--------------------------------------------------------------------------

SET(engine_files 
	debug.h
	file_watcher.h
	file_watcher.cc
	input.h
	input.cc
	window.h
	window.cc
	stb_image.h
	texture.h
	texture.cc
	render_mesh.h
	render_mesh.cc
	default_meshes.h
	default_meshes.cc
	file_io.h
	file_io.cc
	shader.h
	shader.cc
	camera.h
	camera.cc
	sdf.h
	sdf.cc
	hit_result.h
	collider.h
	collider.cc
	rigidbody.h
	rigidbody.cc
	physics_world.h
	physics_world.cc
)
SOURCE_GROUP("engine" FILES ${engine_files})
ADD_LIBRARY(engine STATIC ${engine_files})
ADD_DEPENDENCIES(engine glew glfw)
TARGET_INCLUDE_DIRECTORIES(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(engine PUBLIC exts glew glfw ${OPENGL_LIBS})

