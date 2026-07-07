#pragma once

namespace Mupfel {

    /**
     * @brief Component that stores per-entity movement parameters.
     *
     * The Movement component is used by the Movement System to update
     * an entity's Transform each frame. It defines the linear and angular
     * velocity of an entity, as well as optional acceleration and friction
     * parameters that allow smooth, natural motion.
     *
     * This component is tightly aligned to 16 bytes, as it is stored as a
     * GPU buffer.
     */
    struct alignas(16) Movement {

        /**
         * @brief Linear velocity along the X axis.
         *
         * The Movement System adds this velocity (scaled by timestep)
         * to the entity’s Transform position every update.
         */
        float velocity_x = 0.0f;

        /**
         * @brief Linear velocity along the Y axis.
         *
         * Used in the same way as @ref velocity_x by the Movement System.
         */
        float velocity_y = 0.0f;

        /**
         * @brief Linear velocity along the Z axis.
         *
         * Allows entities to move in full 3D space when desired.
         */
        float velocity_z = 0.0f;

        /**
         * @brief Angular velocity around the Z axis.
         *
         * This value represents radians per second.
         * The Movement System applies it to modify the entity's rotation.
         */
        float angular_velocity = 0.0f;

        /**
         * @brief Initial acceleration applied when movement begins.
         *
         * This optional parameter lets the Movement System gradually
         * ramp up the velocity when a movement command is executed.
         * If unused, keep this value at 0.
         */
        float initial_acceleration = 0.0f;

        /**
         * @brief A decay factor applied to the acceleration each frame.
         *
         * This value represents how much the acceleration decreases per
         * second.
         */
        float acceleration_decay = 0.0f;

        /**
         * @brief Linear friction applied every frame.
         *
         * If non-zero, this value represents a negative acceleration to
         * simulate friction.
         */
        float friction = 0.0f;

    private:
        /**
         * @brief Padding to maintain 16-byte alignment for GPU friendliness.
         *
         * This value has no functional meaning and is used only for memory
         * alignment in SIMD and SSBO contexts.
         */
        float _pad0 = 0.0f;
    };
}
