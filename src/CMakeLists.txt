set(MT_TRANSLATOR_SOURCES mt-translator.c input_utils.c pipe_event_dispatcher.c)

if(HAVE_LINUX_UINPUT_H)
	list(APPEND MT_TRANSLATOR_SOURCES uinput_event_dispatcher.c)
endif(HAVE_LINUX_UINPUT_H)

add_executable(mt-translator ${MT_TRANSLATOR_SOURCES})
target_link_libraries(mt-translator ${MTDEV_STATIC_LIBRARIES})
