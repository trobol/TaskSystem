find_package(Threads REQUIRED)
add_library(TaskSystem
    Task.cpp
    Pool.cpp
    Worker.cpp
    Engine.cpp
    TaskQueue.cpp
)

