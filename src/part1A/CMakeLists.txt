# Application Name
set( APP_NAME   "part1A" )

add_executable( ${APP_NAME} )

target_sources( ${APP_NAME}
    PRIVATE
      ../common/main.xc
      ${APP_NAME}.c
      ../common/filters/filter_coef_double.c
)

target_link_libraries( ${APP_NAME} app_common )

target_compile_options( ${APP_NAME} PRIVATE ${APP_SHARED_COMPILE_OPTIONS} )

target_compile_definitions( ${APP_NAME}
    PRIVATE
      APP_NAME="${APP_NAME}"
      INPUT_WAV="${INPUT_WAV_PATH}"
      OUTPUT_WAV="${WORKSPACE_PATH}/out/output-${APP_NAME}.wav"
      OUTPUT_JSON="${WORKSPACE_PATH}/out/${APP_NAME}.json"
)

target_link_options( ${APP_NAME} PRIVATE ${APP_SHARED_LINK_OPTIONS} )

install(TARGETS ${APP_NAME} DESTINATION ${WORKSPACE_PATH}/bin )


