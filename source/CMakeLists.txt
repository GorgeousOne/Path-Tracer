add_executable(example
	example.cpp
        ../framework/shape.hpp ../framework/shape.cpp
        ../framework/sphere.hpp ../framework/sphere.cpp
        ../framework/box.hpp ../framework/box.cpp
		../framework/triangle.hpp ../framework/triangle.cpp
		../framework/composite.hpp ../framework/composite.cpp

        ../framework/ray.hpp
        ../framework/hitPoint.hpp
        ../framework/material.hpp ../framework/material.cpp
        ../framework/scene.hpp ../framework/scene.cpp
        ../framework/light.hpp
        ../framework/pointLight.hpp
        ../framework/camera.hpp
        ../framework/printVec3.hpp ../framework/printVec3.cpp

        ../framework/pixel.hpp ../framework/pixel.cpp
        ../framework/pixel.hpp ../framework/ppmwriter.cpp
        ../framework/renderer.hpp ../framework/renderer.cpp
	)

target_link_libraries(example ${FRAMEWORK_NAME} ${LIBRARIES})
add_dependencies(example glfw ${FRAMEWORK_NAME} ${COPY_BINARY})

install(TARGETS example DESTINATION .)

add_executable(raytracer raytracer.cpp)

target_link_libraries(raytracer ${FRAMEWORK_NAME} ${LIBRARIES}
        glfw
        ${GLFW_LIBRARIES}
        )
add_dependencies(raytracer glfw ${FRAMEWORK_NAME} ${COPY_BINARY})

install(TARGETS raytracer DESTINATION .)

# Build tests
add_executable(tests
        tests.cpp
        ../framework/shape.hpp ../framework/shape.cpp
        ../framework/sphere.hpp ../framework/sphere.cpp
        ../framework/box.hpp ../framework/box.cpp
		../framework/triangle.hpp ../framework/triangle.cpp
		../framework/composite.hpp ../framework/composite.cpp

		../framework/ray.hpp
        ../framework/hitPoint.hpp
        ../framework/material.hpp ../framework/material.cpp
        ../framework/scene.hpp ../framework/scene.cpp
        ../framework/light.hpp
        ../framework/pointLight.hpp
        ../framework/camera.hpp
        ../framework/printVec3.hpp ../framework/printVec3.cpp

        ../framework/pixel.hpp ../framework/pixel.cpp
        ../framework/pixel.hpp ../framework/ppmwriter.cpp
        ../framework/renderer.hpp ../framework/renderer.cpp
        )
target_link_libraries(tests
        ${GLFW_LIBRARIES}
        )

