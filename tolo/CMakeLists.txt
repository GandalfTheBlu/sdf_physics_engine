#--------------------------------------------------------------------------
# tolo
#--------------------------------------------------------------------------

SET(tolo_files 
	common.h
	code_builder.h
	code_builder.cpp
	expression.h
	expression.cpp
	file_io.h
	file_io.cpp
	lex_node.h
	lex_node.cpp
	lexer.h
	lexer.cpp
	parser.h
	parser.cpp
	program_handle.h
	program_handle.cpp
	token.h
	tokenizer.h
	tokenizer.cpp
	virtual_machine.h
	virtual_machine.cpp
)
SOURCE_GROUP("tolo" FILES ${tolo_files})
ADD_LIBRARY(tolo STATIC ${tolo_files})
TARGET_INCLUDE_DIRECTORIES(tolo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

