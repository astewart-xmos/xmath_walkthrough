# Application Name
set( APP_NAME   "appA2" )

add_executable( ${APP_NAME} )

target_sources( ${APP_NAME}
    PRIVATE
      ./main.c
      ./fft_float.c
      ./fft_bfp.c
      ./fft_wrapped.c
      ./floating_fft.c
)

target_link_libraries( ${APP_NAME} 
    lib_xcore_math
)

target_compile_options( ${APP_NAME} PRIVATE ${APP_SHARED_COMPILE_OPTIONS} )

target_compile_definitions( ${APP_NAME}
    PRIVATE
      APP_NAME="${APP_NAME}" 
)

target_link_options( ${APP_NAME} PRIVATE ${APP_SHARED_LINK_OPTIONS} )

install(TARGETS ${APP_NAME} DESTINATION ${WORKSPACE_PATH}/bin )


