include_guard()

function(copy_compile_commands)
  set(output ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../compile_commands.json)
  add_custom_target(copy_compile_commands ALL DEPENDS ${output})
  add_custom_command(
    OUTPUT
      ${output}
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
      ${output}
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile commands to: ${output}"
    VERBATIM
  )
endfunction()

function(link_compile_commands)
  set(output ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../compile_commands.json)
  add_custom_target(link_compile_commands ALL DEPENDS ${output})
  add_custom_command(
    OUTPUT
      ${output}
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${output}
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    COMMENT "Linking compile commands to: ${output}"
    VERBATIM
  )
endfunction()
