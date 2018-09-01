ADD_EXECUTABLE(MiniBoard resources.c _main.c brush.c canvas.c gui.c)
TARGET_LINK_LIBRARIES(MiniBoard ${GTK3_LIBRARIES})
