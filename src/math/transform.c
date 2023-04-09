void tf_set(struct transform *tf, float position[3], float rotation[3],
        float scale[3])
{
    tf->position.c.x = position[0];
    tf->position.c.y = position[1];
    tf->position.c.z = position[2];

    tf->rotation.c.x = rotation[0];
    tf->rotation.c.y = rotation[1];
    tf->rotation.c.z = rotation[2];

    tf->scale.c.x = scale[0];
    tf->scale.c.y = scale[1];
    tf->scale.c.z = scale[2];
}
